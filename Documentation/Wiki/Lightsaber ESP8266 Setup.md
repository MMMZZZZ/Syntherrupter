# Lightsaber ESP8266 Setup
## Introduction
This page covers how to get the ESP8266 (in the following: ESP) ready to work in the lightsabers. To build the lightsabers themselves, please have a look at the Schematic and Wiring folder.

The lightsabers use ESP8266 to communicate with the Syntherrupter. They are cheap and are supposed to work even in a noisy environment like a tesla coil. 

The ESPs require their own firmware which has to be flashed once. Ideally they shouldn't require updates if there're new features, because they are not involved in the actual processing of the sensor data. This is completely handled by the Tiva microcontroller. Being realistic, they'll still get updates because in a week I'll find something to improve or to fix. 

## Requirements
* [Python](https://www.python.org/downloads/). Installation is pretty straight forward. Make sure to check the option "Add Python to PATH". 
* [esptool](https://github.com/espressif/esptool). Installation is again pretty easy. Simply run the following in your command line: `python -m pip install esptool`. It will install the tool and everything needed to run it. 
* If not included on your ESP board, a serial to USB converter. 

## Flashing
* Extract the ESP firmware from the [latest Syntherrupter release](https://github.com/MMMZZZZ/Syntherrupter/releases/latest). The file is called "Syntherrupter_Lightsaber.ino.generic.bin". 
* Connect the ESP to your PC. You might have to install the drivers for your programmer manually. The cheap ones usually use a CH340 Serial to USB converter chip. 
* Open a command line, type `esptool.py write_flash 0x0 "PATH_TO_THE_EXTRACTED_BINARY"`. It will scan all serial ports until it finds an ESP and uploads the firmware. You can speed up the scan part by specifing the port. Example when connected to COM3: `esptool.py -p COM3 write_flash 0x0 "PATH_TO_THE_EXTRACTED_BINARY"`.
* That's it!

## Lightsaber ID setting
For the Syntherrupter and for you it is important to distinguish each lightsaber. Therefore you have to assign a unique ID to each one of them. By default they all have the ID 1. Two lightsabers with the same ID will be treated as one. Their sensor data will be mixed together and you can only turn both on and off at the same time. 

To set the ID, connect the ESP you want to use in the lightsaber to the Syntherrupter. Enter lightsaber mode and hold the ID you want to assign to this ESP. It will be stored immediately in the ESPs flash, meaning you can immediately disconnect the ESP from the Syntherrupter after holding the button. And you only need to do this once for each lightsaber. 

While you can set an ID for the receiving ESP that will stay connected to the Syntherrutper, it is not important. The ID is only used by those ESPs which act as transmitter (see below).

## Transmitters, Receivers, Pairing
You might wonder how to configure an ESP for being a transmitter sitting in a lightsaber or the receiver connected to the Syntherrutper. Simple answer: you don't. Since only the ESPs in the lightsabers are actually connected to a MPU-6050 sensor they can recognize themselves what their role is. If a MPU-6050 is detected, the ESP will wait for the receiving ESP to power up and connect to it. 

Usually the pairing happens within seconds after power up. A successfull connection is visible by the blue LED on the ESPs lighting up. Sometimes the MPU-6050 fails to initialize and thus the ESP thinks he is a receiver, not a transmitter. In this case a power cycle usually solves the problem. 

Even more rare: for some unknown reason the receiving ESP sometimes stops working. Haven't found the reason yet. Again, a power cycle should solve the issue. 

Other than that pairing happens automatically. If the connection is lost, it should reestablish in less than a second. In my tests the connection was pretty solid, with only minor hickups every couple minutes. However that was without tesla coil... 
