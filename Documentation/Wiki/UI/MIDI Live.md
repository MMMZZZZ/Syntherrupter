# MIDI Live Mode

[Back to the User Interface Reference page](README.md#readme)

MIDI Live mode allows you to play MIDI data received over any of Syntherrupters serial inputs (USB or MIDI In jack). It also contains options to control how this data is processed and passed to the individual tesla coils. 

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [Note Ontime](#note-ontime)
	* [Note Duty](#note-duty)
	* [Apply to Outputs and MIDI Coil Settings](#apply-to-outputs-and-midi-coil-settings)
	* [Applying Manually, On Release or Immediately](#applying-manually-on-release-or-immediately)
	* [NRP Reset](#nrp-reset)
	* [Return to Main Menu](#return-to-main-menu)
* [Sysex Commands](#sysex-commands)

## What you see

![MIDI Live Mode](/Documentation/Pictures/UI/MIDI%20Live.png)

## What you get

### Note Ontime

Using the slider you can vary the nominal ontime of a single note within the [user's limits](Users.md#readme) The nominal ontime is the ontime you'll see on the output if ADSR/envelope is disabled, the MIDI volume is at max. and the note velocity is at max. 

### Note Duty

Using the slider you can vary the nominal duty of a single note within the [user's limits](Users.md#readme). Same principle as the note ontime. 

The reason for having an ontime and a duty setting at the same time is to allow "maximum" power across the entire note frequency range. Syntherrupter evaluates for each note what preset (ontime or duty) will result in the highest ontime, and uses this value. This effectively means that low notes will play at the duty setting specified and high notes play at the given ontime. 

If you want all notes to play at the same ontime, simply set the duty slider to 0. Similarly you can let all notes play at constant duty by setting the ontime to 0.

### Apply to Outputs and MIDI Coil Settings

Whenever you change a setting it will affect the outputs selected here. You'll note that the values below the buttons change to your current settings. More details about this is explained on the [Simple mode](Simple.md#apply-to-outputs) page. 

Unlike the information displayed in [Simple mode](Simple.md#apply-to-outputs), the displayed value can exceed the [Coil Limits](Coil%20Limits.md#readme). That is because it is the nominal ontime and nominal duty per note, *before* any of the various MIDI effects got applied. 99% of the time you'll see values smaller than these on the output. As a reminder, the actual safety mechanisms are applied as a last step to the output signal. They work independently of what you see on the display.

Keeping any of the buttons pressed to access the [MIDI Coil Settings](Coil%20Settings.md#readme) page. From there you'll be able to access the [MIDI Channel Settings](Channel%20Settings.md#readme), too. If you only have a single output, you'll see a "Channels|Pan" button instead which brings you to the same page.

### Applying Manually, On Release or Immediately

Works in the same way as described here: [Simple Mode](Simple.md#applying-manually-on-release-or-immediately).

### NRP Reset

Reset all [Non-Registered Parameters](/Documentation/Wiki/Custom%20MIDI%20Commands.md#non-registered-parameters-nrp) of all channels. 

On Syntherrupter NRPs are not affected by the channel controllers reset command. Those reset commands are usually sent at the beginning and end of a MIDI File. If the NRPs are included in the MIDI file, this wouldn't be an issue, however, if you entered them by hand, you'd have to reenter them every time you start or stop playing a MIDI file. 

### Return to Main Menu

Works in the same way as described here: [Simple Mode](Simple.md#return-to-main-menu).

## Sysex Commands

Ontime and active outputs can be controller by the [Common Mode Parameters](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x20-0x3f-common-mode-parameters) group of the sysex commands. 

MIDI Live mode specific settings are available through the [MIDI Live Mode Parameters](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x60-0x7f-midi-live-mode-parameters) group of the sysex commands.

The apply mode can be changed using the [UI Settings](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x220-0x23f-ui-settings) sysex commands.
