# Custom MIDI Commands

[Back to the wiki main page](README.md#readme)

Syntherrupter's MIDI functions can be controlled via custom MIDI commands. Since vendor or even device specific MIDI commands are part of the MIDI standard, embedding them in your MIDI files does not break the compatibiliy of the file with other devices. 

## Index

* [Custom MIDI Commands](#custom-midi-commands)
* [Index](#index)
	* [Non-registered Parameters (NRP)](#non-registered-parameters-nrp)
		* [NRP Format](#nrp-format)
		* [Syntherrupters NRPs](#syntherrupters-nrps)
	* [System Exclusive Messages (SysEx)](#system-exclusive-messages-sysex)
		* [Quick Start](#quick-start)
		* [Technical Introduction](#technical-introduction)
		* [SysEx Format](#sysex-format)
		* [Syntherrupters SysEx Commands Version 1 (Draft!)](#syntherrupters-sysex-commands-version-1-draft)

## Non-registered Parameters (NRP)

Non-registered parameters allow control over up to 16383 controllers, each one with 14 bit resolution. These commands are not "vendor encoded" like SysEx messages (see below) therefore Syntherrupter only uses controller numbers that are not known to be used by any manufacturer. 

If you've never used NRPs or Syntherrupters advanced features, you might have a look at the following helpful links:
 * [Explanation about NRPs and how to use them](https://www.recordingblogs.com/wiki/midi-registered-parameter-number-rpn)
 * [Syntherrupters stereo features in words and videos](https://highvoltageforum.net/index.php?topic=1020.msg8343#msg8343)

NRPs are implemented via control change commands, which means that the controlled parameter does not affect the same parameter on another MIDI channel - just like other controllers like pedals, volume, pan, ...

### NRP Format

There are 4 controller messages that are used to modify NRPs:
 * Controller 99 - NRP number coarse (NC): Sets the upper 7 bit of the NRP number
 * Controller 98 - NRP number fine (NF): Sets the lower 7 bit of the NRP number
 * Controller 6 - Data entry coarse (DC): Sets the upper 7 bit of the current NRP
 * Controller 38 - Data entry fine (DF): Sets the lower 7 bit of the current NRP

All 4 commands are independant, meaning a NC command does not affect the lower 7 bits of the current NRP number. They have whatever value has previously been assigned to them.

After finishing the modification of NRPs, the NRP number should be reset to 0x7f7f.

### Syntherrupters NRPs

Currently Syntherrupter has support for the following NRPs (all values are decimal, not hex):
|Parameter|NC|NF|DC|DF|
|-|-|-|-|-|
|Stereo Mapping, Control|42|0|X (Don't care)|Stereo Mapping Mode (see below)|
|Stereo Mapping, Input Range|42|1|0-127: upper end|0-127: lower end|
|Stereo Mapping, Output Range|42|2|0-127: upper end|0-127: lower end|

#### Stereo Mapping Modes

Syntherrupter allows you to map notes to stereo positions based on different parameters. 
* Mode 0: "Off". Disables Mapping completely. 
* Mode 1: "Individual". Each note gets mapped based on its pitch (note number + pitch bend + tuning)
* Mode 2: "Omni". All notes of this channel play on all outputs equally. The stereo position of the outputs is completely ignored by this channel. 
* Mode 3-6: Because chords can cover a large pitch range, they appear on multiple outputs at the same time. This quickly leads to "everything being everywhere". The visual effect of precise voices moving around is gone. Just one big mess with some peaks here and there. These modes try to solve this issue by assigning all notes of a channel the same position. Now each channel has again a "sharp" position on the stereo scale.
	* Mode 3: "Lowest". All notes of the channel get mapped based on the current lowest pitch.
	* Mode 4: "Highest". All notes of the channel get mapped based on the current highest pitch.
	* Mode 5: "Average". All notes of the channel get mapped based on the current average pitch.
	* Mode 6: "Loudest". All notes of the channel get mapped based on the pitch of the note with the highest velocity. If multiple notes have the same velocity, the pitch of the most recent one is taken.

## System Exclusive Messages (SysEx)

### Quick Start

If you are a regular user, you only need to know the following three things:
1. Sysex are MIDI messages that allow manufacturers and people like me to completely control their devices over MIDI. This allows f.ex. to embed the entire device setup into a MIDI file and thus "hands free" operation. 
2. **The only practical way to use this extremely powerful and useful feature is a tool called [Syfoh](https://github.com/MMMZZZZ/Syfoh#readme)**. It allows you to control Syntherrupter with simple sentences like *set ontime for coil 1 and mode simple to 100*. You can just write all your commands into a text file and Syfoh will apply them with one click. 
3. Once you figured out Syfoh, this page only serves you as a reference which commands exist and what they do. Syfoh contains a list of the commands, too, but the actual documentation of them lives here, [further down the page](#syntherrupters-sysex-commands-version-1-draft). Note: the documentation (especially the [Conventions](#conventions) section) below contains more info than you need, but most of it is highly relevant for you, too. F.ex. you can ignore that float commands all have a command number >= `0x2000` but you should know what the [NF] tag means, or how ranges work with floats, etc.

From now on the documentation goes into all the technical details that'd be required to write your own compatible program.  

### Technical Introduction

SysEx messages were originally intended to allow manufacturers to implement completely custom commands within the MIDI protocol*. Part of each command is a device manufacturer ID (DMID) to make sure the commands of different manufacturers don't interfere with each other. Hence, with your own ID you can implement whatever you want. 

SysEx Messages have no channel information and are intended to control the device as a whole. Although, of course, every manufacturer can use them to implement whatever they want. Custom note on messages with additional information for example. 

*\*With GM2, kind of a revision and extension of the MIDI standard, "Universal SysEx Messages" have been introduced. They have an ID just like any other SysEx message but as the "universal" indicates, they are well defined and intended to be implemented on any suitable device. It's basically a backward compatible extension of the existing MIDI standard. As of now, Syntherrupter has no support for Universal SysEx Messages (and tbh they're quite rare in MIDI files).*

### SysEx Format

#### General Format

The MIDI Standard only defines how SysEx messages start and end. The actual content is manufacturer specific. Here's a good explanation of the overall format: http://personal.kent.edu/~sbirch/Music_Production/MP-II/MIDI/midi_system_exclusive_messages.htm

In a nutshell, the standard looks like this:
|Byte Nr.|Value|Meaning|
|-|-|-|
|1|0xf0|Beginning of a SysEx message|
|2|0x00|2-byte DMID following|
|3|0x00-0x7f|Upper byte of the DMID|
|4|0x01-0x7f|Lower byte of the DMID|
|5-n|0x00-0x7f|Zero or more manufacturer specific data bytes|
|n+1|0xf7|End of a SysEx message|

#### Syntherrupters Format

Synhterrupter SysEx messages are roughly inspired by [Rolands format](https://www.2writers.com/eddie/TutSysEx.htm). They are at most 16 bytes long, which is handy because the UART FIFOs of the Tiva microcontroller are 16 bytes large. 

Structure: 
```F0 00 [DMID] [DMID] [version] [device ID] [PN LSB] [PN MSB] [TG LSB] [TG MSB] [value LSB] [value] [value] [value] [value MSB] F7```
* `DMID`: I decided to use the manufacturer ID `26 05` (hex). It seems to be far enough in the european block to be sure that it won't be assigned to any manufacturer in the next years. *Note: getting a unique ID from the MMA (association behind MIDI) costs at least 240$/year. Surprisingly cheap but still too expensive for a niche project like this.*
* `version`: Protocol version. Allows for future revisions that could break compatibility. Anything after this byte can potentially change with a new version.
* `device ID`: Assuming multiple devices on the same MIDI bus, this allows addressing them individually. 127 means broadcast.
* `PN`: Parameter number. Defines the meaning of the command. Note: unlike the parameter value, the parameter number is not split into 7 bit groups. Meaning, that the next PN after `0x7f` is `0x100` (`PN = (PN MSB << 8) + PN LSB`).
* `TG`: Target. Specifies to what the command applies. Taking the example of an ontime command, the target specifies what coil is affected. 
* `value`: Parameter value. With 5 MIDI data bytes, full 32 bit values can be covered. Any 32bits of data will be sent in groups of 7 bits, LSB first. 

*Note: Since not every parameter requires the full 9 bytes, it will be possible to send shorter messages which omit some of those bytes. However, they aren't standarized yet.*

### Syntherrupters SysEx Commands Version 1 (Draft!)

#### Use [Syfoh](https://github.com/MMMZZZZ/Syfoh#readme)!

Just in case you missed the [bold part above](#quick-start)... 

#### This is still a draft!

Currently these commands are only implemented in beta firmware versions. Since the version number is a single integer from 1-127 I can't name this version `0.1` or `1.0-beta.1`. Hence this note. This protocol shall be considered as draft and can change at any time until a stable firmware is released. At that point this note will disappear. 

Also, keep in mind that there is a ton of parameters that has to be documented here, checked and implemented in the firmware and in [Syfoh](https://github.com/MMMZZZZ/Syfoh#readme). That's a looot of sources for errors and oversights... Let me know if you find any oopsies!

#### Conventions

All conventions are to be read as "unless noted otherwise... ".

* Target value 127 is reserved for broadcasting, meaning it will affect all targets (works for LSB/MSB independently).
* Reserved target bytes are expected to be 0 or 127.
* Parameters and parameter options that are currently not supported by Syntherrupter are marked by an [NS].
* Parameters that stored in EEPROM are marked by an [EE]. Other parameters will be reset to default after a power cycle.
* Parameters usually are read- and writable. Read- and write-only commands are marked by [RO] and [WO] respectively.
* Every integer parameter has a float version at offset `0x2000`. 
	* Float values in this documentation are always written with the C-style `f`-suffix to indicate that it are 32-bit floats (`xx.xf`).
	* If the integer parameter is expressed as fractional (fixed point) value, the float parameter is not. Example: 
		* Integer command is in 1/1000
		* Sending 1.2% as integer 0.012\*1000 => `12`
		* Sending 1.2% as float => `0.012f` (NOT `12.0f`)
		* Benefit: the float version is independent of the integer resolution. 
	* If the integer parameter covers a certain range, the float parameter is expected to cover that range with a value between 0.0f and 1.0f. Example:
		* Integer range goes from 0 to 127
		* Sending 1/4 of the range as integer: 127/4 => `32`
		* Sending 1/4 of the range as float: `0.25f` (NOT `32.0f`)
		* Benefit: the float version is independent of the integer range size/resolution
	* Commands without float version are marked with [NF]. Examples are the envelope steps. Since these need to be discrete values, a float version doesn't make sense. 
* The parameter value can be one of the following types:
	* int32
	* float32
	* char[4]
	* bitfield, noted as bf8, where 8 would indicate that the field is 8 bits wide (starting at the least significant bit of the parameter value). A bitfield range within the bitfield is noted as [LSB-MSB], f.ex. [2-7]
* Any parameter value or part of it that is not specified by this document is reserved.
* Any parameter that is not compliant with these specs shall be ignored.

#### Overview

The commands are grouped by purpose. Any command (range) that's not listed here should be considered as reserved.
* [`0x01-0x1f`: System commands](#0x01-0x1f-system-commands). Commands like a read request or an answer to a read request
* [`0x20-0x3f`: Common mode parameters](#0x20-0x3f-common-mode-parameters). Things like ontime, which are to some extend required by all modes. 
* [`0x40-0x5f`: Simple mode parameters](#0x40-0x5f-simple-mode-parameters). Control specific properties of Simple mode.
* [`0x60-0x7f`: MIDI Live mode parameters](#0x60-0x7f-midi-live-mode-parameters). Control specific properties of MIDI Live mode.
* [`0x100-0x10f`: Lightsaber mode parameters](#0x100-0x10f-lightsaber-mode-parameters). Control specific properties of Lightsaber mode.
* `0x200-0x3ff`: Settings
	* [`0x200-0x21f`: EEPROM and other control commands](#0x200-0x21f-eeprom-and-other-control-commands)
	* [`0x220-0x23f`: UI settings](#0x220-0x23f-ui-settings)
	* [`0x240-0x25f`: User settings](#0x240-0x25f-user-settings)
	* [`0x260-0x27f`: Coil settings](#0x260-0x27f-coil-settings)
	* [`0x300-0x31f`: Envelope settings](#0x300-0x31f-envelope-settings)
	
#### `0x01-0x1f`: System commands

* `0x01`: Response to request
	* Target MSB: Target MSB that has been requested.
	* Target LSB: Target LSB that has been requested.
	* Value: Value of the requested parameter.
* `0x02`: Request if parameter is supported
	* Target MSB: 0 or any target value you want to check if it's supported.
	* Target LSB: 0 or any target value you want to check if it's supported.
	* Value: uint, parameter number or range (see command `0x04` below) to check. The response will be a `0x01` response message with one of the following values:
		* 0: Not supported
		* 1: Supported
* `0x03`: Read Parameter. Get reply as `0x01` response message.
	* Target MSB: Target MSB to read for the given parameter. Wildcards are supported (resulting in multiple responses).
	* Target LSB: Target LSB to read for the given parameter. Wildcards are supported (resulting in multiple responses).
	* Value: uint, Parameter number or range (see command `0x04` below) to read. 
* `0x04`: Get Parameter(s). Get reply as normal command. F.ex. reading parameter `0x20` would return a `0x20` command with the current settings.
	* Target MSB: Target MSB to read for the given parameter. Wildcards are supported (resulting in multiple responses).
	* Target LSB: Target LSB to read for the given parameter. Wildcards are supported (resulting in multiple responses).
	* Value: char[4], parameter number or range to read. When leaving the last parameter number to 0 only the rage start parameter number is read. Otherwise all parameters in the range will be read. When reading a range of parameters, the last number is included within the range.
		* [0]: First parameter number LSB
		* [1]: First parameter number MSB
		* [2]: Last parameter number LSB (optional)
		* [3]: Last parameter number MSB (optional)

#### `0x20-0x3f`: Common mode parameters

* `0x20`: [NF] Mode Enable
	* Target MSB: uint, target mode
		* 1: Simple Mode
		* 2: MIDI Live Mode
		* 3: Lightsaber mode
	* Target LSB: Reserved.
	* Value: int32
		* 0: Disable mode (default)
		* 1: Enable mode
* `0x21`: Ontime
	* Target MSB: uint, target mode
		* 1: Simple Mode
		* 2: MIDI Live Mode
		* 3: Lightsaber mode
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, ontime in us
		* Default: 0
* `0x22`: Duty cycle
	* Target MSB: uint, target mode
		* 1: Simple Mode
		* 2: MIDI Live Mode
		* 3: [NS] Lightsaber mode
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, duty cycle in 1/1000
		* Default: 0
* `0x23`: BPS
	* Target MSB: uint, target mode
		* 1: Simple Mode
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, BPS in Hz
		* Default: 0
* `0x24`: Period
	* Target MSB: uint, target mode
		* 1: Simple Mode
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, period in us

#### `0x40-0x5f`: Simple mode parameters

* `0x40`: Ontime Filter Factor
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, factor value in 1/1000
		* Default: 2000 (2.0)
* `0x41`: Ontime Filter Constant
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, factor value in 1/1000 (default: 0)
		* Default: 30000 (30.0)
* `0x44`: BPS Filter Factor
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, factor value in 1/1000
		* Default: 1800 (1.8)
* `0x45`: BPS Filter Constant
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, factor value in 1/1000
		* Default: 5000 (5.0)

#### `0x60-0x7f`: MIDI Live mode parameters

* `0x60`: MIDI Channels assigned to Coil
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: bf16, marking every channel as assigned (1) or unassigned (0) for this coil.	
		* Default: 0xffff (all channels assigned)
* `0x61`: [NS] Coils assigned to MIDI Channel
	* Target MSB: Reserved.
	* Target LSB: uint, target MIDI channel
		* 0-15. 
	* Value: bf6, marking every coil as assigned (1) or unassigned (0) for this MIDI channel. Bitfield size limited by your firmware if you flashed a binary for less outputs.
		* Default: 0x3f (all coils assigned)
* `0x62`: Pan Configuration
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: bf3
		* [0-2]: uint, Reach mode.
			* 0: Constant
			* 1: Linear (default)
* `0x63`: Pan Position
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32
		* 0-127: Pan position, 0=left, 127=right
		* All other values: Stereo features disabled (default)
* `0x64`: Pan Reach
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32
		* 0-127: Pan reach
* `0x66`: Reset Channel NRPs
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32
		* bf16: channels whose NRPs shall be reset to default.
* `0x67`: [EE] LFO Modulation Depth
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 0-127: Modulation depth. 0=no modulation, 127=full depth
		* Default: 63
* `0x68`: [EE] LFO Frequency
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32, Frequency in 1/1000
* `0x69`: [EE] LFO BPM
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32, Beats per minute.

#### `0x100-0x10f`: Lightsaber mode parameters

* `0x100`: Assigned Lightsabers
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: bf4, marking each lightsaber as assigned (1) or not (0)
		* Default: 0x0f (all lightsabers assigned)
* `0x101`: [NF] Set Lightsaber ID
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 0-4: New ID for the connected ESP8266.  Will be remembered by the ESP8266.

#### `0x200-0x21f`: EEPROM and other control commands

* `0x200`: [EE] EEPROM Update Mode
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 0: Manual mode (default)
		* 1: Force update, does not affect current update mode.
		* 2: Auto, update EEPROM after each command/change.
* `0x201`: [EE] Device ID
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 0-126: New ID for this device.
		* Default: 0
* `0x202`: Reset
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32, reset key in 1/1000. To prevent an accidental reset of the device, a specific value must be sent. 
		* 41153700: Causes a reset of the Tiva microcontroller and in consequence of the Nextion display. Note: this doesn't work when any [passthrough mode](/Documentation/UI/Nextion-USB.md#readme) is active.

#### `0x220-0x23f`: UI settings

* `0x220`: [EE] Display brightness
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 0-100: Brightness in percent
		* Default: 100
* `0x221`: [EE] Standby
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 0: Standby disabled (default)
		* 1-3600: Seconds until Syntherrupter goes into standby
* `0x222`: [EE] UI Button Hold Time
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 50-9999: Milliseconds to hold a button until alternate function is activated.
		* Default: 250
* `0x223`: Safety Options
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: bf1
		* [0]: Background shutdown
			* 0: Disabled
			* 1: Enabled (default)
* `0x224`: [NS] [EE] [NF] UI Color Mode
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32, color mode
		* 0: Light colors
		* 1: Dark colors (default)
* `0x225`: [NF] UI Apply Mode
	* Target MSB: uint, target mode.
		* 127: wildcard, affects all modes. 
	* Target LSB: Reserved.
	* Value: int32
		* 0: Manually
		* 1: On Release (default)
		* 2: Immediately

* `0x226`: [NF] UI Update Mode
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32, Specify when the Nextion UI is updated after receiving sysex commands. Similar to the [EEPROM Update Mode command](#0x200-0x21f-eeprom-and-other-control-commands). However, the [Nextion UI is not always capable of receiving updates](/Documentation/UI/Menu.md##sysex-commands). In this case the update is discarded. Disabling auto updates speeds up the execution significantly, thus reduces the risk of audible hickups when sending sysex commands. 
		* 0: Manual updates, sysex commands will not affect on-screen data.
		* 1: [NS] Force update, if possible, update all outdated values. Does not affect current update mode. 
		* 2: Auto (default), if possible, sysex commands do immediately update the Nextion data. 

#### `0x240-0x25f`: User settings

* `0x240`: [EE] User Name
	* Target MSB: uint, char group position within target string. When setting char group 0 the string will be deleted (set to `\x00`). Hence the null-termination does not need to be sent explicitly. 
		* 0-7.  No broadcasting.
	* Target LSB: uint, user
		* 0-2. No broadcasting.
	* Value: char[4]
* `0x241`: [EE] User Password
	* Target MSB: uint, char group position within target string. When setting char group 0 the string will be deleted (set to `\x00`). Hence the null-termination does not need to be sent explicitly. 
		* 0-7. No broadcasting.
	* Target LSB: uint, user
		* 0-2. No broadcasting.
	* Value: char[4]
* `0x242`: [EE] User Max Ontime
	* Target MSB: Reserved.
	* Target LSB: uint, user
		* 0-2. No broadcasting.
	* Value: int32, ontime in us
* `0x243`: [EE] User Max Duty
	* Target MSB: Reserved.
	* Target LSB: uint, user
		* 0-2. No broadcasting.
	* Value: int32, duty in 1/1000
* `0x244`: [EE] User Max BPS
	* Target MSB: Reserved.
	* Target LSB: uint, user
		* 0-2. No broadcasting.
	* Value: int32, BPS in Hz

#### `0x260-0x27f`: Coil settings

* `0x260`: [EE] Coil Max Ontime
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, ontime in us
		* Default: 10
* `0x261`: [EE] Coil Max Duty
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, duty in 1/1000
		* Default: 50 (0.05)
* `0x262`: [EE] Coil Min Ontime. Important: the minimum ontime is an offset that gets added to every ontime *after* applying the duty limiter but *before* the ontime limiter. It doesn't affect the minimum offtime either. As a consequence, higher minimum ontimes in combination with many voices and/or high frequencies will likely exceed the coils duty limit.
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, ontime in us
		* Default: 0
* `0x263`: [EE] Coil Min Offtime
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, offtime in us
		* Default: 10
* `0x264`: [EE] [NF] Coil Max MIDI Voices
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
		* 0-5. Limited by your firmware if you flashed a binary for less outputs.
	* Value: int32, voice limit
		* 0-16
		* Default: 8
* `0x265`: [NS] [EE] Coil Output Invert
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
	* Value: int32
		* 0: Normal (ontime = 3.3V, offtime = 0V) (default)
		* 1: Inverted (ontime = 0V, offtime = 3.3V)
* `0x266`: [EE] Buffer Duration
	* Target MSB: Reserved.
	* Target LSB: Reserved.
	* Value: int32
		* 1000-100000: Duration of the output buffer in us. Same for all coils.
		* Default: 5000
* `0x267`: [RO][NF] Active Tones
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
	* Read value: int32, number of currently active tones (of all modes together) on this output.
* `0x268`: [RO] Output Signal Duty Cycle
	* Target MSB: Reserved.
	* Target LSB: uint, target coil
	* Read value: int32, duty cycle of the output signal in 1/1000.

#### `0x300-0x31f`: Envelope settings

* `0x300`: [EE] [NF] Envelope Next Step
	* Target MSB: uint, program number
		* 0-63
	* Target LSB: uint, step number
		* 0-7
	* Value: int32, number of next step
		* 0-7
* `0x301`: [EE] Envelope Step Amplitude
	* Target MSB: uint, program number
		* 0-63
	* Target LSB: uint, step number
		* 0-7
	* Value: int32, amplitude in 1/1000
* `0x302`: [EE] Envelope Step Duration
	* Target MSB: uint, program number
		* 0-63
	* Target LSB: uint, step number
		* 0-7
	* Value: int32, duration in us
* `0x303`: [EE] Envelope Step n-tau
	* Target MSB: uint, program number
		* 0-63
	* Target LSB: uint, step number
		* 0-7
	* Value: int32, n-tau in 1/1000
