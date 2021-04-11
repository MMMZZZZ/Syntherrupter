# Lightsaber ESP8266 Setup

[Back to the wiki main page](README.md#readme)

The lightsabers use ESP8266s to communicate with the Syntherrupter. They are cheap and supposed to work even in a noisy environment like a tesla coil. 

This page covers how to get those ESP8266 (in the following: ESP) ready to work. This requires the circuitry described on the [Lightsaber Receiver](HW/Lightsaber%20Receiver.md#readme) wiki page. To build the lightsabers themselves, please have a look at the pictures [here](/Documentation/Wiring%20and%20Schematics/Lightsaber%20Sender).

The ESPs require their own firmware which has to be flashed once. Ideally they shouldn't require updates if there're new features, because they are not involved in the actual processing of the sensor data. This is completely handled by the Tiva microcontroller. 

## Index
* [Index](#index)
* [Flashing for Windows Users](#flashing-for-windows-users)
* [Flashing with cross-plattform tool](#flashing-with-cross-plattform-tool)
* [Lightsaber ID setting](#lightsaber-id-setting)
* [Transmitters, Receivers, Pairing](#transmitters-receivers-pairing)

## Flashing for Windows Users
For Windows exists a graphical tool for flashing the ESPs. 

*Some of the following steps and pictures are taken from [Allen Derushas how-to](https://github.com/aderusha/HASwitchPlate/blob/master/Documentation/01_Arduino_Sketch.md).*

### Requirements
* [Latest Syntherrupter release](https://github.com/MMMZZZZ/Syntherrupter/releases/)
* [NodeMCU Flasher](https://github.com/nodemcu/nodemcu-flasher/) ([32bit download](https://github.com/nodemcu/nodemcu-flasher/raw/master/Win32/Release/ESP8266Flasher.exe) / [64bit download](https://github.com/nodemcu/nodemcu-flasher/raw/master/Win64/Release/ESP8266Flasher.exe)). 

### Steps
* Extract the ESP firmware from the Syntherrupter release zip file. The file is called "Syntherrupter_Lightsaber.ino.generic.bin". 
* Connect Syntherrupter to your computer and choose *ESP<->USB* in Syntherrupters *Settings* menu. 
* Connect your ESP to Syntherrupter. Make sure that the ESP GPIO0 pin is connected to ground.
* Launch the NodeMCU flasher and select the COM port for your device from the *COM Port* drop-down menu
* Select the *Config* tab (1), then click the top-most gear icon to the right (2) to open a file browser. ![NodeMCU_Flasher_OpenFirmware](https://github.com/aderusha/HASwitchPlate/blob/master/Documentation/Images/NodeMCU_Flasher_OpenFirmware.png?raw=true) 
* Switch back to the *Operation* tab and click *Flash(F)*.
* Disconnect the ESP from Syntherrupter, connect the next one, and click again on *Flash(F)*. Repeat for all ESPs.
* Once all ESPs are flashed, power cycle Syntherrupter to leave ESP<->USB Passthrough mode. 

## Flashing with cross-plattform tool
You have to use the Python command line tool from Espressiv. But don't worry, it's very easy. 1 command to install the tool, 1 command to upload the firmware.

### Requirements
* [Latest Syntherrupter release](https://github.com/MMMZZZZ/Syntherrupter/releases/)
* [Python](https://www.python.org/downloads/). Installation is pretty straight forward. Make sure to check the option "Add Python to PATH". 
* [esptool](https://github.com/espressif/esptool). Installation is again pretty easy. Simply run the following in your command line: `python -m pip install esptool`. It will install the tool and everything needed to run it. 

### Steps
* Extract the ESP firmware from the Syntherrupter release zip file. The file is called "Syntherrupter_Lightsaber.ino.generic.bin". 
* Connect Syntherrupter to your computer and chose *ESP<->USB* in Syntherrupters *Settings* menu. 
* Connect your ESP to Syntherrupter. Make sure that the ESP GPIO0 pin is connected to ground. 
* Open a command line, type `esptool.py write_flash 0x0 "PATH_TO_THE_EXTRACTED_BINARY"`. It will scan all serial ports until it finds an ESP and uploads the firmware. You can speed up the scan part by specifing the port. Example when connected to COM3: `esptool.py -p COM3 write_flash 0x0 "PATH_TO_THE_EXTRACTED_BINARY"`.
* Repeat the last two steps for all of your ESPs.
* Once all ESPs are flashed, power cycle Syntherrupter to leave ESP<->USB Passthrough mode. 

## Lightsaber ID setting
For the Syntherrupter and for you it is important to distinguish each lightsaber. Therefore you have to assign a unique ID to each one of them. By default they all have the ID 1. Two lightsabers with the same ID will be treated as one. Their sensor data will be mixed together and you can only turn both on and off at the same time. 

To set the ID, connect the ESP you want to use in the lightsaber to the Syntherrupter. Enter lightsaber mode and hold the ID you want to assign to this ESP. It will be stored immediately in the ESPs flash, meaning you can immediately disconnect the ESP from the Syntherrupter after holding the button. And you only need to do this once for each lightsaber. 

While you can set an ID for the receiving ESP that will stay connected to the Syntherrupter, it is not important. The ID is only used by those ESPs which act as transmitter (see below).

## Transmitters, Receivers, Pairing
You might wonder how to configure an ESP for being a transmitter sitting in a lightsaber or the receiver connected to the Syntherrupter. Simple answer: you don't. Since only the ESPs in the lightsabers are actually connected to a MPU-6050 sensor they can recognize themselves what their role is. If a MPU-6050 is detected, the ESP will wait for the receiving ESP to power up and connect to it. 

Usually the pairing happens within seconds after power up. A successfull connection is visible by the blue LED on the ESPs lighting up. Sometimes the MPU-6050 fails to initialize and thus the ESP thinks he is a receiver, not a transmitter. In this case a power cycle usually solves the problem. 

Even more rare: for some unknown reason the receiving ESP sometimes stops working. Haven't found the reason yet. Again, a power cycle should solve the issue. 

Other than that pairing happens automatically. If the connection is lost, it should reestablish in less than a second. In my tests the connection was pretty solid, with only minor hickups every couple minutes. However that was without tesla coil... 
