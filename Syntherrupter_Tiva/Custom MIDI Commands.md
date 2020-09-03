# Custom MIDI Commands
Syntherrupter's MIDI functions can be controlled via custom MIDI commands. Since vendor or even device specific MIDI commands are part of the MIDI standard, embedding them in your MIDI files does not break the compatibiliy of the file with other devices. 

## Non-registered Parameters (NRP)
Non-registered parameters allow control over up to 16383 controllers, each one with 14 bit resolution. These commands are not "vendor encoded" like SysEx messages (see below) therefore Syntherrupter only uses controller numbers that are not known to be used by any manufacturer. 

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
|Stereo Mapping, Control|42|0|X (Don't care)|0: Off (normal stereo), 1: On, 2: Omni Mode|
|Stereo Mapping, Input Range|42|1|0-127: upper end|0-127: lower end|
|Stereo Mapping, Output Range|42|1|0-127: upper end|0-127: lower end|

### Recommended Links
If you've never used NRPs or Syntherrupters advanced features, you might have a look at the following helpful links:
 * [Explanation about NRPs and how to use them](https://www.recordingblogs.com/wiki/midi-registered-parameter-number-rpn)
 * [Syntherrupters stereo features in words and videos](https://highvoltageforum.net/index.php?topic=1020.msg8343#msg8343)

## System Exclusive Messages (SysEx)
SysEx messages were originally intended to allow manufacturers to implement completely custom commands within the MIDI protocol. Part of each command is a device manufacturer ID (DMID) to make sure the commands of different manufacturers don't interfer with each other. Hence, with your own ID you can implement whatever you want. 

However with GM2, kind of a "revision and extension" of the MIDI standard, "Universal SysEx Messages" were introduced. They have a ID just like any other SysEx message but as the "universal" indicates, they are well defined and intended to be implemented on any suitable device. It's basically a backward compatible extension of the existing MIDI standard. As of now, Syntherrupter has no support for Universal SysEx Messages (and tbh they're quite rare in MIDI files).

SysEx Messages have no channel information and are intended to control the device as a whole. Although, of course, every manufacturer can use them to implement whatever they want. Custom note on messages with additional information for example. 

### SysEx format
Here's a good explanation of the format: http://personal.kent.edu/~sbirch/Music_Production/MP-II/MIDI/midi_system_exclusive_messages.htm

Short explanation:
|Byte Nr.|Value|Meaning|
|-|-|-|
|1|0xf0|Beginning of a SysEx message|
|2|0x00|2-byte DMID following|
|3|0x00-0x7f|Upper byte of the DMID|
|4|0x01-0x7f|Lower byte of the DMID|
|5-n|0x00-0x7f|Zero or more manufacturer specific data bytes|
|n+1|0xf7|End of a SysEx message|

### Syntherrupters SysEx Messages
**Currently Syntherrupter does not support any SysEx Messages. This is work in progress.**

I decided to use the manufacturer ID `0x26 0x05`. It seems to be far enough in the european block to be sure that it won't be assigned to any manufacturer in the next years. 

*Note: getting a unique ID from the MMA (association behind MIDI) costs at least 240$/year. Surprisingly cheap but still too expensive for a niche project like this.*

Each message begins with a message ID, specifying the message's content. Since all message bytes are MIDI data bytes, they all range from 0x00 to 0x7f. 

List of messages:
* ID 0x00: Reserved
* ID 0x01: Reserved for GUI control messages (documented in separate file). Following bytes would be passed to the GUI Input buffer and processed there. 
* ID 0x02: ADSR data point (ADSR-DP) set.
	* Data bytes
		* Byte 1: ADSR-DP number
		* Byte 2: ADSR-DP type
			* 0x00: constant
			* 0x01: linear
			* 0x02: exponential
		* Byte 3: ADSR-DP amplitude upper 7 bit
		* Byte 4: ADSR-DP amplitude lower 7 bit
		* Byte 5: ADSR-DP duration upper 7 bit
		* Byte 6: ADSR-DP duration upper-middle 7 bit
		* Byte 7: ADSR-DP duration lower-middle 7 bit
		* Byte 8: ADSR-DP duration lower 7 bit
		* Byte 9: ADSR-DP n-tau upper 7 bit
		* Byte 10: ADSR-DP n-tau lower 7 bit
	* Number formats
		* Amplitude
			* 14 bit unsigned fixed-point (14bit value will be divided by 2^7 = 0x80 = 128). 
			* It is a volume *factor*, meaning that a value of 1 leaves the volume unchanged. Amplitude of the last data point should be 0.
			* Range is 0 - 127.9922 with 0.0078 resolution
		* Duration
			* 28 bit unsigned integer. 
			* It indicates the duration in microseconds it takes since the last datapoint until the amplitude of this datapoint is reached. 
			* Range is 0us - 268s with 1us resolution
		* n-tau
			* 14 bit signed fixed point (14bit value will be divided by 2^6 = 0x40 = 64). 
			* It expresses the number of time constants (tau) that will be fitted inside the given duration. A value of 0 is equivalent to a linear curve. The bigger the value the more curved the curve gets. Eventually, for n=inf it would be rectangular. The sign indicates whether the curve is concave (positive sign) or convex (negative sign). 
			* Range is -128 - 127.98 with 0.016 resolution.
