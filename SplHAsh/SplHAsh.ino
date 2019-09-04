/*   
 * SplHAsh
 * An MQTT based sprinkler controller running on ESP3266
 * Licensed under the MIT License, Copyright (c) 2019 TJPoorman
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// Mapping NodeMCU Ports to Arduino GPIO Pins
// Allows use of NodeMCU Port nomenclature in config.h
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12 
#define D7 13
#define D8 15

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const boolean static_ip = STATIC_IP;
IPAddress ip(IP);
IPAddress gateway(GATEWAY);
IPAddress subnet(SUBNET);

const char* mqtt_broker = MQTT_BROKER;
const char* mqtt_clientId = MQTT_CLIENTID;
const char* mqtt_username = MQTT_USERNAME;
const char* mqtt_password = MQTT_PASSWORD;
const char* mqtt_topic_base = MQTT_TOPIC_BASE;

String clientBase = MQTT_CLIENTID;
String availabilitySuffix = "/availability";
String availabilityTopicStr = clientBase + availabilitySuffix;
const char* availabilityTopic = availabilityTopicStr.c_str();
const char* birthMessage = "online";
const char* lwtMessage = "offline";

const int relayActive = RELAY_ACTIVE;

const int dataPin = SR_DATA_PIN;	//Outputs the byte to transfer
const int loadPin = SR_LATCH_PIN;	//Controls the internal transference of data in SN74HC595 internal registers
const int clockPin = SR_CLOCK_PIN;	//Generates the clock signal to control the transference of data

const int maxZone = MAX_ZONE;
const int pumpZone = PUMP_ZONE;
const int shiftRegisterCount = floor(maxZone / 8.0001) + 1;
int pumpSR = 0;
int pumpSRPin = 0;

WiFiClient espClient;
PubSubClient client(espClient);


/***************************************************
 * Setup/Loop Functions
****************************************************/
void setup() {  
  pinMode(dataPin, OUTPUT);
  pinMode(loadPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  // Set the Shift Register for pump if defined
  if (pumpZone != -1) {
    pumpSR = floor(pumpZone / 8.0001) + 1;
    pumpSRPin = (pumpZone % 8 == 0) ? 8 : pumpZone % 8;
  }

  // Setup serial output, connect to wifi, connect to MQTT broker, set MQTT message callback
  Serial.begin(115200);
  Serial.println("Starting SplHAsh...");

  if (relayActive == HIGH) {
    Serial.println("Relay mode: Active-High");
  }
  else {
    Serial.println("Relay mode: Active-Low");
  }

  // Set all zones off to start
  ActivateZone(0);
  
  setup_wifi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);
}

void loop() {
  // Connect/reconnect to the MQTT broker and listen for messages
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Wifi setup function
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (static_ip) {
    WiFi.config(ip, gateway, subnet);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print(" WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}


/***************************************************
 * MQTT Functions
****************************************************/
// Callback when MQTT message is received; calls triggerZoneAction(), passing topic and payload as parameters
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

  String topicToProcess = topic;
  payload[length] = '\0';
  String payloadToProcess = (char*)payload;
  triggerZoneAction(topicToProcess, payloadToProcess);
}

// Function that publishes birthMessage
void publish_birth_message() {
  Serial.print("Publishing birth message \"");
  Serial.print(birthMessage);
  Serial.print("\" to ");
  Serial.print(availabilityTopic);
  Serial.println("...");
  client.publish(availabilityTopic, birthMessage, true);
}

// Function that runs in loop() to connect/reconnect to the MQTT broker, and publish the current zone statuses on connect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_clientId, mqtt_username, mqtt_password, availabilityTopic, 0, true, lwtMessage)) {
      Serial.println("Connected!");

      // Publish the birth message on connect/reconnect
      publish_birth_message();

      for (int j = 0; j <= maxZone; j++) {
        // Subscribe to the action topics to listen for action messages
        String topic = mqtt_topic_base + String(j) + "/action";
        Serial.print("Subscribing to ");
        Serial.print(topic);
        Serial.println("...");
        client.subscribe(topic.c_str());
      }

      // Publish the current zone status(s) on connect/reconnect to ensure status is synced with whatever happened while disconnected
      for (int j = 0; j <= maxZone; j++) {
        publish_zone_status(j, "OFF");
      }
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/***************************************************
 * Zone Functions
****************************************************/
// Function called by callback() when a message is received 
// Passes the message topic as the "topic" parameter and the message payload as the "requestedAction" parameter
void triggerZoneAction(String topic, String requestedAction) {
  bool zoneFound = false;
  int zoneActivated;
  
  // Loop through all zones until the called topic matches the zones
  for (int i = 0; i <= maxZone; i++) {
    if (topic == mqtt_topic_base + String(i) + "/action") { 
      zoneFound = true;
      zoneActivated = i;
      break;
    }
  }

  if (zoneFound) {
    if (zoneActivated == 0) {		// If zone0 is called turn all zones off
      ActivateZone(0);
      for (int i = 0; i <= maxZone; i++) {
        publish_zone_status(i, "OFF");
      }
    }
    else {
      if (requestedAction == "ON") {
        Serial.print("Triggering ");
        Serial.print(topic);
        Serial.println(" ON!");
        ActivateZone(zoneActivated);
        for (int i = 0; i <= maxZone; i++) {
          if (i == zoneActivated) {
            publish_zone_status(i, "ON");
          }
          else { publish_zone_status(i, "OFF"); }
        }
      }
      else if (requestedAction == "OFF") {
        Serial.print("Triggering ");
        Serial.print(topic);
        Serial.println(" OFF!");
        ActivateZone(0);
        for (int i = 0; i <= maxZone; i++) {
          publish_zone_status(i, "OFF");
        }
      }
      else { Serial.println("Unrecognized action payload... taking no action!"); }
    }
  }
  else { Serial.println("Unrecognized action topic... taking no action!"); }
}

// Function that checks zone status and publishes an update when called
void publish_zone_status(int zoneNum, String state) {
    client.publish(String(mqtt_topic_base + String(zoneNum) + "/status").c_str(), state.c_str(), true);
}

// Function to activate a specific zone.
// Works by getting the byte needed for each
// shift register and shifting them out.
void ActivateZone(int zoneNum) {
  byte data;
  
  digitalWrite(loadPin, LOW);
  for(int i = 1; i <= shiftRegisterCount; i++) {
    if (zoneNum > 0) {
      data = getByteForShiftRegister(i, (floor(zoneNum / 8.0001) + 1 == i) ? ((zoneNum % 8 == 0) ? 8 : zoneNum % 8) : 0);
    }
    else {
      data = (relayActive == LOW) ? 255 : 0;
    }
    
    shiftOut(dataPin, clockPin, data);
  }
  digitalWrite(loadPin, HIGH);
}


/***************************************************
 * Shift Register Functions
****************************************************/
// Function to determine they byte needed for the given shift register
byte getByteForShiftRegister(int shiftRegister, int shiftRegisterPin) {
  byte result = 0;
  
  if(shiftRegisterPin > 0 || pumpSR == shiftRegister) {
    for(int i=7; i>=0; i--){
      if (i + 1 == shiftRegisterPin || (shiftRegister == pumpSR && i + 1 == pumpSRPin)){
        result |= relayActive << i;
      }
      else {
        result |= !relayActive << i;
      }
    }
  }
  
  return result;
}

// Function to send the byte data to they shift register
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {  
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}
