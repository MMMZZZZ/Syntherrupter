# Settings

[Back to the User Interface Reference page](README.md#readme)

From this page you can access all Settings. Except for a few (marked) exceptions, all of the settings are remembered between power cycles.

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [General Settings](#general-settings)
	* [User Settings](#user-settings)
	* [Envelope Settings](#envelope-settings)
	* [Coil Limits](#coil-limits)
	* [Nextion<->USB](#nextion-usb)
	* [ESP<->USB](#esp-usb)
	* [Save/Return to Main Menu](#savereturn-to-main-menu)

## What you see

![Settings](/Documentation/Pictures/UI/Settings.png)

## What you get

### General Settings

Opens the [General Settings](General.md#readme). Allows you to control general aspects of the user interface, like screen brightness, time till display standby, etc.

### User Settings

Opens the [User Settings](Users.md#readme). Specify usernames, passwords and limits.

### Envelope Settings

Opens the [Envelope Settings](Envelope.md#readme). Edit any of the MIDI envelopes including the presets. 

### Coil Limits

Opens the [Coil Limits](Coil%20Limits.md#readme). Set the "absolute maximum ratings" for every single of the up to 6 coils. 

### Nextion<->USB

Enters [Nextion<->USB Passthrough Mode](Nextion-USB.md#readme). This allows direct serial communication through USB with the Nextion display, f.ex. for [firmware flashing](/Documentation/Wiki/Firmware%20Flashing.md#nextion-flashing-over-usbserial).

### ESP<->USB

Enters [ESP8266<->USB Passthrough Mode](Nextion-USB.md#readme). This allows direct serial communication through USB with the Nextion display, f.ex. for [firmware flashing](/Documentation/Wiki/Lightsaber%20ESP8266%20Setup.md#flashing-for-windows-users).

### Save/Return to Main Menu

Instead of the usual *Return* label, the button is labeled with *Save* even though it brings you back to the [main menu](Menu.md#readme). The reason is that only when pressing this button Syntherrupter will actually write the new Settings to the EEPROM. In case you did anything wrong and don't want to save them, just power cycle and your changes will be gone. 
