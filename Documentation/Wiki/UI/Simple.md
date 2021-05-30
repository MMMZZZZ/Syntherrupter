# Simple Mode

[Back to the User Interface Reference page](README.md#readme)

Simple mode allows you to generate classic, single tone interrupter signals. You have control over ontime, BPS and duty of the signal. Simple mode includes a slight filter that causes a smooth transition when changing any of the settings. As of now this cannot be disabled or modified without recompiling.

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [Ontime](#ontime)
	* [BPS](#bps)
	* [Duty](#duty)
	* [Apply to Outputs](#apply-to-outputs)
	* [Applying Manually, On Release or Immediately](#applying-manually-on-release-or-immediately)
	* [Return to Main Menu](#return-to-main-menu)
* [Sysex Commands](#sysex-commands)

## What you see

![Simple Mode](/Documentation/Pictures/UI/Simple.png)

## What you get

### Ontime

Using the slider you can vary the ontime within the [user's limits](Users.md#readme). 

Any change automatically updates the duty value, too.

### BPS

Using the slider you can vary the BPS (Bans Per Second, frequency) within the [user's limits](Users.md#readme).

Any change automatically updates the ontime or duty value, too. Your most recent change will be preserved. F.ex. if you set the ontime, then adjust the BPS, you ontime setting will be preserved and the duty value adjusted according to your changes. 

### Duty

Using the slider you can vary the duty cycle within the [user's limits](Users.md#readme).

Any change automatically updates the ontime value, too.

### Apply to Outputs

This part of the UI is only visible if you have more than one output. 

Each output is represented by a button. This button doesn't show whether the output is active, but whether your next change will affect this output. If you want to change the outputs 1 and 3 you select these two, and deselect all the other outputs. Any change you make will only affect those two outputs. The others continue to operate on whatever settings they currentley are.

Since it is hard to remember 6 sets of ontime and BPS (and duty), the currently active values are displayed below each button. 

Unlike the values displayed in [MIDI Live mode](MIDI%20Live.md#apply-to-outputs-and-midi-coil-settings), these values do respect the [Coil Limits](Coil%20Limits.md#readme). Here it makes sence because you'll see your settings 1:1 on the output(s); there are no musical effects. 

### Applying Manually, On Release or Immediately

*This section applies equally to all modes (Simple, [MIDI Live](MIDI%20Live.md#readme) and [Lightsaber](Lightsaber.md#readme)). Hence you may have been redirected to this section from a different mode.*

By default new values are applied *On Release* to the selected outputs, meaning that they get applied as soon as you release any of the three sliders. 

Pressing the *On Release* button will change the mode to *Manually* and another button appears: *Apply Now*. This gives the user the control when exactly he wants to apply new settings and allows f.ex. to change multiple values at the same time. 

Pressing the *Manually* button changes the mode again. The third and last mode applies changes *Immediately*. This means that your slider movement will affect the outputs in real time. 

### Return to Main Menu

*This section applies equally to all modes (Simple, [MIDI Live](MIDI%20Live.md#readme) and [Lightsaber](Lightsaber.md#readme). Hence you may have been redirected to this section from a different mode.*

A simple press of the *Return* button stops Simple mode and brings the user back to the [Main Menu](Menu.md#readme). 

Keeping the button pressed brings the user back to the [Main Menu](Menu.md#readme), too. However, Simple mode won't be stopped. It continues to run in the background. There are no limitations to this feature. Any combination of modes can run simultaneously.

## Sysex Commands

Ontime and active outputs can be controller by the [Common Mode Parameters](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x20-0x3f-common-mode-parameters) group of the sysex commands. 

Simple mode specific settings are available through the [Simple Mode Parameters](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x40-0x5f-simple-mode-parameters) group of the sysex commands.

The apply mode can be changed using the [UI Settings](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x220-0x23f-ui-settings) sysex commands.
