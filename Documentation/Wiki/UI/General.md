# General Settings

[Back to the Settings Menu page](Settings.md#readme)

The General Settings mainly control user interface stuff.

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [Button hold time](#button-hold-time)
	* [Display Brightness](#display-brightness)
	* [Enter Sleep after](#enter-sleep-after)
	* [Background shutdown](#background-shutdown)
	* [Dark Mode](#dark-mode)
* [Sysex Commands](#sysex-commands)

## What you see

![General Settings](/Documentation/Pictures/UI/General.png)

## What you get

### Button hold time

Control how long you need to keep a button pushed to trigger its alternate function. Default value is 250ms. Depending on your [display model](/Documentation/Wiki/Minimum%20Viable%20Setup.md#suitable-models) you can reduce it. This is however not recommended for the basic models; it will be hard to know when a touch was long enough or not because the loading times of the display are in the same order of magnitude. 

### Display Brightness

Allows you to dim the backlight of the screen. 

### Enter Sleep after

To save energy you can allow and control the displays sleep mode. A value of 0:00 disables sleep mode; the display will always stay on. Any other value will cause the display to shut itself down after the given time. It wakes up when you touch it.

### Background shutdown

**Important: For safety reasons this parameter is not stored in EEPROM** 

By default, touching any part of the background, or any "passive" component like text shuts down all of Syntherrupters outputs ([more details](/Documentation/Wiki/Getting%20Started.md#emergency-stop)). You can disable this behavior by setting the value to `0`. However, after a power cycle it will be activated again. 

### Dark Mode

Enables or disables Dark Mode. Dark Mode is better suited for dark environments, however it offers less contrast (blue-white is higher contrast than blue-black).

## Sysex Commands

All of the settings above can be controlled by the [UI settings](Custom%20MIDI%20Commands.md#0x220-0x23f-ui-settings) group of the sysex commands. 
