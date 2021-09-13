# Startup

[Back to the User Interface Reference page](README.md#readme)

Let's get this party started!

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
  * [Black Screen](#black-screen)
  * [Syntherrupter Title](#syntherrupter-title)
  * [Firmware Versions](#firmware-versions)
  * [Infos and Warnings](#infos-and-warnings)

## What you see

![Startup](/Documentation/Pictures/UI/Startup.png)

## What you get

### Black Screen

During the initialization - which includes a handshake between the Tiva microcontroller and the Nextion display as well as the loading of all settings from EEPROM - the screen will stay black. This usually only takes one second. If the black screen doesn't disappear you likely have an issue with your setup (wrong wiring, insufficient power supply, ...).

### Syntherrupter Title

Let's face it, every startup screen somehow shows a logo or brand or product name. Why not? If you see the Syntherrupter logo/name the initialization was successful. 

### Firmware Versions

At the bottom of the screen you'll see what firmware versions are installed on the Nextion display and the Tiva microcontroller. While some firmware versions are backwards compatible you really shouldn't mix the versions. 

### Infos and Warnings

Syntherrupter will show you a warning instead of the Syntherrupter Title when settings have been migrated from a previous, incompatible version. In case something goes wrong, it shows a warning as well. 

#### EEPROM Upgrade

Sometimes a new version requires new or different values to be stored in the EEPROM. In that case the old settings are loaded and new settings get initialized to default values. The firmware is thus fully functional (unless noted otherwise in the release notes) while preserving the previous content of the EEPORM. This allows you to go back to a previous firmware if necessary. Once you're sure the new firmware works fine and that all settings have been loaded correctly, you can update the EEPROM by hitting "Save" on the [Settings page](Settings.md#savereturn-to-main-menu) or by using the corresponding [Sysex command](https://github.com/MMMZZZZ/Syntherrupter/blob/dev/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x200-0x21f-eeprom-and-other-control-commands). It's only now that the new - incompatible - settings get written to the EEPROM (and thus can't be loaded by previous versions anymore). 

#### No Settings found

If your Syntherrupter is all new, you'll get a warning that the EEPROM doesn't contain valid settings - what a surprise. Syntherrupter will automatically load default settings which you can check, change and save in the [Settings](Settings.md#readme) menu.

#### Incompatible Settings found

Syntherrupter is usually able to migrate older settings versions to the current version. However it is technically impossible to correctly read future versions. Thus, if you previously had a newer firmware installed, you may see a warning that Syntherrupter found settings it can't handle. In this case default settings are loaded again. Just as with the upgrade process detailed above, the actual content of the EEPROM remains unchanged until you instruct Syntherrupter to save something to it. You can thus go back to a previous version without being forced to lose any data (though you can't use it). 
