# Lightsaber Receiver

[Back to the hardware documentation overview](README.md#readme)

The Lightsaber feature is based on the ESP8266. To flash them and to receive data in normal operation you need to be able to connect them to the Tiva LaunchPad. 

To see how to build the Lightsabers themselves, please have a look at the pictures in the following [folder](/Documentation/Wiring%20and%20Schematics/Lightsaber%20Sender).

## Communication Pins

* Tiva PA6: ESP GPIO1 (U0TX)
* Tiva PA7: ESP GPIO3 (U0RX)

## Schematic and Pinout

The following schematic of a "complete" Syntherrupter shows at the bottom right how to connect the ESP8266 to the Tiva LaunchPad. Note that you need some kind of switch for GPIO2. When powering up the ESP this pin determines if it can be flashed or if it operates normally. Since you need to flash all ESPs I suggest to include a female pin header for connecting the ESPs. 

![Complete Schematic](/Documentation/Wiring%20and%20Schematics/Syntherrupter%20Complete/Syntherrupter%20Complete%20Schematic.png)

ESP-01 Pinout: 

![ESP-01 Pinout](/Documentation/Wiring%20and%20Schematics/ESP-01%20Pinout.png)