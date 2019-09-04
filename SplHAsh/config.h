/*
 * SplHAsh
 * An MQTT based sprinkler controller running on ESP3266
 * Licensed under teh MIT License, Copyright (c) 2019 TJPoorman
 *
 * User-configurable Parameters 
*/

// Wifi Parameters
#define WIFI_SSID "your-ssid-name"
#define WIFI_PASSWORD "your-ssid-password"

// Static IP Parameters
#define STATIC_IP false
#define IP 192,168,1,100
#define GATEWAY 192,168,1,1
#define SUBNET 255,255,255,0

// MQTT Parameters
#define MQTT_BROKER "w.x.y.z"
#define MQTT_CLIENTID "SplHAsh"
#define MQTT_USERNAME "your-mqtt-username"
#define MQTT_PASSWORD "your-mqtt-password"
#define MQTT_TOPIC_BASE "yard/sprinkler/"

// Relay Parameters
#define RELAY_ACTIVE HIGH	// Define whether the relay activates HIGH or LOW.

// Sprinkler Zone Parameters
#define MAX_ZONE 8		// Maximum number of zones configured
#define PUMP_ZONE -1		// If your system requires a pump, define the zone here. If not, use -1.

// Shift Register Parameters
#define SR_DATA_PIN D6
#define SR_LATCH_PIN D7
#define SR_CLOCK_PIN D8