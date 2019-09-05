# SplHAsh
### An MQTT based sprinkler controller running on ESP3266

SplHAsh allows for controlling of your sprinkler system using your MQTT based home automation system.

SplHAsh should be controllable via any home automation software that can configure an MQTT switch or send commands over MQTT, including Home Assistant and OpenHAB.

Sample Home Assistant configuration snippets are provided in this repository to get your sprinklers connected to Home Assistant including sample scripts and automation.

SplHAsh has both hardware and software components. The required hardware components include an ESP8266-based microcontroller (such as the NodeMCU), a shift register, and a relay module. The software component is found in this repo.

One of the benefits of this system is it does not matter if you have 4 sprinkler zones or 100.  Using the shift registers allows for virtually limitless zones.

## Table of Contents

* TODO

## How SplHAsh works...

SplHAsh subscribes to an MQTT topic for each of the zones configured (by default `yard/sprinkler/X/action` where X is the corresponding zone).

When the `ON` payload is received for any of these topics, SplHAsh activates a relay connected to the relevant shift register and pin to activate the sprinkler zone.

When the `OFF` payload is received for any of these topics, SplHAsh deactivates a relay connected to the relevant shift register and pin to deactivate the sprinkler zone.

When the `ON` or `OFF` payload is received for the Zone0 topic (by default `yard/sprinkler/0/action`), SplHAsh will deactivate all zones. 

When a payload to activate/deactivate a zone is received SplHAsh publishes the current status of the zones based on it's memory of the zones.  These messages are published with the "retain" flag set.

SplHAsh also publishes a "birth" message on connection to your MQTT broker, and a "last-will-and-testament" message on disconnection from the broker, so that your home automation software can respond appropriately to SplHAsh being online or offline.

SplHAsh also allows for designating a zone as a 'PUMP' zone that will be activated regardless of what zone is called for.  (See the [Sprinkler Zone Parameters](#sprinkler-zone-parameters) section of this documentation.)


## Hardware

### Bill of Materials

Building SplHAsh to control up to 8 zones requires:

| No. | Qty | Part | Link | Approx Price |
| --- | --- | ---- | ---- | ------------ |
| 1. | 1 | ESP8266-based microcontroller (e.g. NodeMCU) | [Link](https://amzn.to/2MUYsIC) | $ 7.00 |
| 2. | 1 | 8 channel relay module | [Link](https://amzn.to/2HLgE3a) | $ 9.00 |
| 3. | 1 | 74HC595 Shift Register | [Link](https://amzn.to/2ZOSGu2) | $ 6.00 |
| 4. | 1 | 24v AC power adapter | [Link](https://amzn.to/2NQm14M) | $ 14.00 |
| 5. | 1 | AC to DC converter | [Link](https://amzn.to/2NQmzrm) | $ 13.00 |
| 6. | 1 | Mini solderless breadboard (400 tie-point) | [Link](https://amzn.to/2ZMyEAg) | $ 10.00 |
| 7. | | Male-to-female breadboard jumper wires | [Link](https://amzn.to/2MUwe0m) | $ 6.00 |
| 8. | | Male-to-male breadboard jumper wires | | |
| 9. | | Project box or case | | |

**Approximate total cost for major components:** $ 59.00, even less if you are replacing an existing controller (you should be able to reuse the current power adapter).

_Note: You will need 1 shift register for every 8 zones in your setup._

### Building SplHAsh

Connect the breadboard according to this diagram:

![Schematic](/Schematic.png)

_Done!_


## Software

### 1. Set up the Arduino IDE

You will modify the configuration parameters and upload the sketch to the NodeMCU with the Arduino IDE.

1. Download the Arduino IDE for your platform from [here](https://www.arduino.cc/en/Main/Software) and install it.
2. Add support for the ESP8266 to the Arduino IDE by following the instructions under "Installing with Boards Manager" [here](https://github.com/esp8266/arduino).
3. Add the "PubSubClient" library to the Arduino IDE: follow the instructions for using the Library Manager [here](https://www.arduino.cc/en/Guide/Libraries#toc3), and search for and install the PubSubClient library.
4. You may need to install a driver for the NodeMCU for your OS - Google for instructions for your specific microcontroller and platform, install the driver if necessary, and restart the Arduino IDE.
5. Select your board from `Tools -> Boards` in the Arduino IDE (e.g. "NodeMCU 1.0 (ESP-12E Module)").

### 2. Load the sketch in the Arduino IDE and modify the user parameters in config.h

SplHAsh's configuration parameters are found in config.h. After loading SplHAsh.ino, select the config.h tab in the Arduino IDE. This section describes the configuration parameters and their permitted values.

_IMPORTANT: No modification of the sketch code in SplHAsh.ino is necessary._

#### Wifi Parameters

`WIFI_SSID "your-wifi-ssid"`
The wifi ssid SplHAsh will connect to. Must be placed within quotation marks.

`WIFI_PASSWORD "your-wifi-password"`
The wifi ssid's password. Must be placed within quotation marks.

#### Static IP Parameters

`STATIC_IP false`
Set to `true` to use the IP / GATEWAY / SUBNET parameters that follow. Set to `false` to use DHCP. _(Default: false)_

`IP 192,168,1,100`
The static IP you want to assign to SplHAsh. _(Default: 192.168.1.100)_

`GATEWAY 192,168,1,1`
The gateway you want to use. _(Default: 192.168.1.1)_

`SUBNET 255,255,255,0`
The subnet mask you want to use. _(Default: 255.255.255.0)_

_Note: There are commas (,) not periods (.) in the IP / GATEWAY / SUBNET parameters above!_

#### MQTT Parameters

`MQTT_BROKER "w.x.y.z"`
The IP address of your MQTT broker. Must be placed within quotation marks.

`MQTT_CLIENTID "SplHAsh"`
The Client ID you want SplHAsh to use. Should be unique among all the devices connected to your broker. Must be placed within quotation marks. _(Default: SplHAsh)_

`MQTT_USERNAME "your-mqtt-username"`
The username required to authenticate to your MQTT broker. Must be placed within quotation marks. Use "" (i.e. a pair of quotation marks) if your broker does not require authentication.

`MQTT_PASSWORD "your-mqtt-password"`
The password required to authenticate to your MQTT broker. Must be placed within quotation marks. Use "" (i.e. a pair of quotation marks) if your broker does not require authentication.

`MQTT_TOPIC_BASE "yard/sprinkler/"`
The base string used for the `action` and `status` topics. _(Default: yard/sprinkler/)_

#### Relay Parameters

`RELAY_ACTIVE HIGH`
Set to `LOW` if using an active-low relay module. Set to `HIGH` if using an active-high relay module. _(Default: HIGH)_

#### Sprinkler Zone Parameters

`MAX_ZONE 8`
The maximum number of zones configured.  This will tell the code how many shift registers are used. _(Default: 8)_

`PUMP_ZONE -1`
The zone that is used for a pump.  This allows for designating a zone that will be activated regardless of what zone is called for.  If no pump is used leave as default. _(Default: -1)_

#### Shift Register Parameters

`SR_DATA_PIN D6`
The GPIO pin connected to the data pin of the shift register. _(Default: NodeMCU D6)_

`SR_LATCH_PIN D7`
The GPIO pin connected to the latch pin of the shift register. _(Default: NodeMCU D7)_

`SR_CLOCK_PIN D5`
The GPIO pin connected to the clock pin of the shift register. _(Default: NodeMCU D8)_

### 3. Upload the sketch to your NodeMCU / microcontroller

If using the NodeMCU, connect it to your computer via MicroUSB. Select `Sketch -> Upload` in the Arduino IDE.

If using a different ESP8266 microcontroller, follow that device's instructions for putting it into flashing/programming mode.

### 4. Check the Arduino IDE Serial Monitor

Open the Serial Monitor via `Tools -> Serial Monitor`. Reset your microcontroller. If all is working correctly, you should see something similar to the following messages:

```
Starting SplHAsh...
Relay mode: Active-High

Connecting to your-wifi-ssid.. WiFi connected - IP address: 192.168.1.100
Attempting MQTT connection...Connected!
Publishing birth message "online" to SplHAsh/availability...
Subscribing to yard/sprinkler/0/action...
Subscribing to yard/sprinkler/1/action...
Subscribing to yard/sprinkler/2/action...
Subscribing to yard/sprinkler/3/action...
Subscribing to yard/sprinkler/4/action...
Subscribing to yard/sprinkler/5/action...
Subscribing to yard/sprinkler/6/action...
Subscribing to yard/sprinkler/7/action...
Subscribing to yard/sprinkler/8/action...

```

If you receive these (or similar) messages, all appears to be working correctly. Disconnect the controller from your computer and prepare to install.

## Installing SplHAsh

1. Install SplHAsh near your sprinkler wires
2. Connect 1 wire from wall plug, 1 wire from the AC side of 5v converter, and the sprinkler common wire together
3. Connect other wire from wall plug, other wire from AC side of 5v converter, and an extra wire together
4. Connect the extra wire to a common terminal of the relay board
5. Daisy-chain all common terminals of the relay board together
6. Connect each zone wire to the corresponding NO terminal on the relay board
7. Connect the positive DC wire of AC/DC converter to the 5v rail of the breadboard (the rail that `vin` pin is connected to)
8. Connect the negative DC wire of AC/DC converter to the ground rail of the breadboard
9. Plug the wall plug in

![Wiring Schematic](/Wiring%20Schematic.png)

_Done!_


## Configuring Home Assistant

SplHAsh supports Home Assistant's "MQTT Switch" platforms.

### Example MQTT Switch Configuration

```
switch:
  - platform: mqtt
    name: "Sprinkler Zone 1"
    state_topic: "yard/sprinkler/1/status"
    command_topic: "yard/sprinkler/1/action"
    availability_topic: "SplHAsh/availability"
    icon: mdi:sprinkler
```

See [Configuration Example](Examples/configuration.yaml) for a full zone setup.

_Note: the "availability_topic" configuration parameter is available in Home Assistant as of version 0.55; it allows Home Assistant to display the switch as "unavailable" if SplHAsh goes offline unexpectedly. When SplHAsh reconnects to your broker, the switch controls will again be available in Home Assistant. SplHAsh forms its availability topic by suffixing "/availability" to the MQTT_CLIENTID parameter in config.h, and publishes "online" to that topic when connecting or reconnecting to the broker and "offline" when disconnecting from the broker._

_Note: the "mdi:sprinkler" icon is available in Home Assistant as of version 0.98.1_

### Example Script

Place the following in your `scripts.yaml`:

```
sprinkler_water_front_side:
  alias: '[Sprinkler] Water Front/Side'
  sequence:
  - data:
      entity_id: switch.sprinkler_zone_1
    service: switch.turn_on
  - delay: 00:30:00
  - data:
      entity_id: switch.sprinkler_zone_2
    service: switch.turn_on
  - delay: 00:15:00
  - data:
      entity_id: switch.sprinkler_zone_off
    service: switch.turn_on
```

This will activate Zone1 for 30 minutes, then activate Zone2 for 15 minutes, then deactivate all zones.

Of course, you can replace the delay with any length of time you wish. Be sure to replace `switch.sprinkler_zone_1` and others with the name of your zones in Home Assistant if different.

See [Scripts Example](Examples/scripts.yaml) for a more examples.

### Example Automation

Place the following in your `automations.yaml`:

```
- id: sprinkler_water_all_zones
  alias: '[Sprinkler] Water All Zones'
  trigger:
  - at: '22:00'
    platform: time
  condition:
  - condition: time
    weekday:
      - mon
      - tue
      - wec
      - thu
      - fri
      - sat
      - sun
  action:
  - data:
      entity_id: script.sprinkler_water_all
    service: script.turn_on
```

This will run the `sprinkler_water_all` script at 10:00PM every day.

Be sure to replace `script.sprinkler_water_all` with the name of your script in Home Assistant if different.

Restart Home Assistant for the configuration, script, and automation changes to take effect.


## Contributing

Fork this repository and submit a pull request.


## Issues/Bug Reports/Feature Requests

Please open an issue in this repository and describe your issue in as much detail as possible with the steps necessary to replicate the bug. It will also be helpful to include the content of your config.h in code tags and indicate whether (and how) you modified SplHAsh.ino.

Please also request new features via issues!


## Credits

A lot of this project is thanks to [marthoc](https://github.com/marthoc) and the [GarHAge](https://github.com/marthoc/GarHAge) project.

Most of the MQTT functions and config settings came from that project.


## License

_Licensed under the MIT License, Copyright (c) 2019 TJPoorman_
