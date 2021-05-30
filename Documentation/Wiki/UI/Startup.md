# Startup

[Back to the User Interface Reference page](README.md#readme)

Let's get this party started!

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get

## What you see

![Startup](/Documentation/Pictures/UI/Startup.png)

## What you get

### Black Screen

During the initialization - which includes a handshake between the Tiva microcontroller and the Nextion display as well as the loading of all settings from EEPROM - the screen will stay black. This usually only takes one second. If the black screen doesn't disappear you likely have an issue with your setup (wrong wiring, insufficient power supply, ...).

### Syntherrupter

Let's face it, every startup screen somehow shows a logo or brand or product name. Why not? If you see the Syntherrupter logo/name the initialization was successful. 

### Firmware Versions

At the bottom of the screen you'll see what firmware versions are installed on the Nextion display and the Tiva microcontroller. While some firmware versions are backwards compatible you really shouldn't mix the versions. 

### Warnings

Syntherrupter will display warnings if something went wrong while loading the settings. 

#### No Settings found

If your Syntherrupter is all new, you'll get a warning that the EEPROM doesn't contain valid settings - what a surprise. Syntherrupter will automatically load default settings which you can check, change and save in the [Settings](Settings.md#readme) menu.

#### Incompatible Settings found

Syntherrupter is usually able to migrate older settings versions to the current version. However it is technically impossible to correctly read future versions. Thus, if you previously had a very old or a newer firmware installed, you may see a warning that Syntherrupter found settings it can't handle. In this case default settings are loaded again. Until you instruct Syntherrupter to update the EEPROM with your current settings you can still flash the previous firmware again and read/use/... the settings.
