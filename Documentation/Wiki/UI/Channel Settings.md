# Channel Settings

[Back to the User Interface Reference page](README.md#readme)

Set NRPs or send any MIDI command for the given MIDI channel. Note: Syntherrupter numerates MIDI channels from 0 to 15. Some programs however numerate them from 1 to 16. 

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [Keypad](#keypad)
	* [NRP Editor](#nrp-editor)
	* [MIDI Command](#midi-command)
	* [Previous/Next Channel](#previousnext-channel)
	* [Return to MIDI Coil Settings](#return-to-midi-coil-settings)
* [Sysex Commands](#sysex-commands)

## What you see

![Channel Settings](/Documentation/Pictures/UI/Channel%20Settings%2015.png)

## What you get

### Keypad

The keypad allows you to modify any of the values in the [NRP Editor](#nrp-editor) and the [MIDI Command](#midi-command) section. A switch to change between hexadecimal and decimal is included. 

By hitting *Ok* the commands are executed. If you want to send only an NRP command or only a MIDI command, you have to enter the "wildcard" values (see below) for the other command.

### NRP Editor

The NRP Editor allows you to quickly set any [Non-Registered Parameter](/Documentation/Wiki/Custom%20MIDI%20Commands.md#non-registered-parameters-nrp). Instead of using MIDI control change commands for all 4 parameters, you can directly enter the values for the NRP number and data entry. 

If you do not want to change all of the 4 parameters, you can use the value `0x80` for those that shall remain unmodified. This can be done for all of the values to not send any NRP command at all.

### MIDI Command

This section allows you to send any 2 or 3-byte MIDI command (no SysEx or realtime messages). If you want to send a command that consists of only 2 bytes (f.ex. MIDI Program Change), use Data 2. Data 1 will be ignored.

If you do not want to send a MIDI command at all, set the status to `0`.

### Previous/Next Channel

The *Previous* (*Next*) button brings you to the previous (next) channel. This allows you to quickly navigate through multiple channels without leaving the page.

### Return to MIDI Coil Settings

Leaving the page brings you back to the [MIDI Coil Settings](Coil%20Settings.md#readme).

## Sysex Commands

The changes on this page are channel specific and done via NRP commands. Thus there are no sysex commands needed/available.
