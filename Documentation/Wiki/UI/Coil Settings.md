# MIDI Coil Settings

[Back to the User Interface Reference page](README.md#readme)

Select what MIDI channels play on this coil and change the coils stereo properties. 

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [Assign MIDI Channels](#assign-midi-channels)
	* [Stereo Settings](#stereo-settings)
		* [Pan Slider](#pan-slider)
		* [Reach Slider and Mode](#reach-slider-and-mode)
	* [Previous/Next Coil](#previousnext-coil)
	* [Return to MIDI Live Mode](#return-to-midi-live-mode)
* [Sysex Commands](#sysex-commands)

## What you see

![Coil Settings](/Documentation/Pictures/UI/Coil%20Settings%204.png)

## What you get

### Assign MIDI Channels

Each MIDI channel can be assigned or unassigned to this output by pushing the respective button. 

Keeping a channel button pressed opens the [Channel Settings](Channel%20Settings.md#readme). 

The dedicated *All* and *None* buttons allow the quick selection and deselection of all channels at once.

### Stereo Settings

This section allows you to control the stereo properties of this coil. If *Pan* is set to *Disabled*, this coil ignores any stereo information and plays normally. 

By switching to *Enabled*, additional settings appear

#### Pan Slider

The pan slider ranges from *L* (left) to *R* (right). It allows you to specify the pan position of this coil. The slider has fixed steps of 1/10th of its range. This makes it very easy to have all coils at the same distance from each other - no matter how many coils you have. 

Examples

* 5 Coils
	* 1/10
	* 3/10
	* 5/10
	* 7/10
	* 9/10
* 6 Coils
	* 0/10
	* 2/10
	* 4/10
	* 6/10
	* 8/10
	* 10/10

#### Reach Slider and Mode

In addition to the position of the coil, a *Reach* needs to be specified. This defines how far the coil "listens" for notes. Notes that are further away from the coils stereo position won't play on this coil anymore. The slider has the same unit and division as the [pan slider](#pan-slider). 

In addition to the coils range limit you can also specify how notes within the range are treated. If reach mode is set to *Const.*, all notes within the coils reach play at their normal volume. This mode is useful if you want that notes always play on one signle coil. Hence you should set the range to half of the distance between coils (no overlapping).

*Linear* on the other hand makes the notes volume decrease with increasing distance. If the notes position is equal with the coils position, the note plays at normal volume. At the edge of the coils range this volume has decreased to 0. This mode is usefull if you want to let voices move seamlessly between coils. In this case the coils reach should equal the distance to the next coil (ranges overlapp). 

### Previous/Next Coil

The *Previous* (*Next*) button brings you to the previous (next) coil. This allows you to quickly navigate through multiple coils without leaving the page.

### Return to MIDI Live Mode

Leaving the page brings you back to the [MIDI Live mode](MIDI%20Live.md#readme) page.

## Sysex Commands

All of the settings above can be controlled by the [MIDI Live Mode Commands](/Documentation/Wiki/Custom%20MIDI%20Commands.md#0x60-0x7f-midi-live-mode-parameters) group of the sysex commands. 
