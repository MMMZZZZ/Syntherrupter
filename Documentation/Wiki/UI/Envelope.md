# Envelope Editor

[Back to the User Interface Reference page](README.md#readme)

The envelope editor allows you to modify any of Syntherrupters envelopes. With envelopes you can control how Syntherrupter sounds. Because of the limitations of Nextion it is not possible to have a graphical editor. However, since you're probably not going to use this on a dayly base, it should be "good enough" and especially better than nothing.

If Syntherrupters envelopes are new to you, you should check out the [Getting Started with Envelopes](/Documentation/Wiki/Tutorials/Getting%20Started%20with%20Envelopes.md#readme). 

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [Program](#program)
	* [Step](#step)
	* [Next Step](#next-step)
	* [Amplitude](#amplitude)
	* [Duration](#duration)
	* [n-tau](#n-tau)
	* [Set Values](#set-values)
	* [Return](#return)
	* [Envelope Examples](#envelope-examples)
  
## What you see

![Envelope Editor](/Documentation/Pictures/UI/Envelope.png)

## What you get

### Program

Select which envelope (or in MIDI terms which program) is being modified. 

### Step

Each envelope contains up to 8 steps. Step 1 is always the one that plays on a `Note On` command, so it is always the "attack" of the note. Step 8 is always the "release", meaning the step that plays when a `Note Off` command is received. Since step 8 is the release, its amplitude is always zero.

### Next Step

Specify what step number follows to the current step. This can be used to create loops (jump back to previous steps), create staccato sounds (jump to last step), or simply to stay on a certain amplitude until release happens (next step equals current step). An example of a looping envelope can be seen [below](#envelope-examples).

### Amplitude

Amplitude is the factor by which the [Note Ontime](MIDI Live.md#note-ontime) will be multiplied. For the first step it can make sense to specify values higher than `1.0`. Actually, you can specify pretty much any value for any step - except the last step. Since it is the note release it is always 0.

### Duration

Duration is how much time it takes to reach the amplitude of the current step. 

### n-tau

Controls the slope of the envelope inbetween two data points. A value of `0.0` gives a linear transition. Positive values make it behave like an RC low pass filter (exponential decay towards target amplitude). A higher value makes it steeper at the beginning ("lower R in the filter"), an infinite value would make it rectangular. Negative values are possible, too. This "mirrors" the slope. Instead of rising fast at first, it now rises fast at the end. The pictures [below](#envelope-examples) should make clear what the parameter does exactly. 

### Set Values

Applies the values you're currently seeing on screen. Unlike most other settings pages where values are applied automatically, you have to do it manually here. This is to prevent accidential overwrites. Note that you have to apply every single step.

### Return

Brings you back to the [Settings Menu](Settings.md#readme). Unlike on other settings pages this does *not* apply any values. You have to use the [Set Values button](#set-values).

### Envelope Examples

The following graph shows the [built-in envelope](#todo) number 8 for different values of [n-tau](#n-tau). Amplitudes and durations were not modified, hence all traces cross in the same 4 points. 

![Envelope n-tau](/Documentation/Pictures/Envelope%20n-tau.png)

The second example shows a simple looping curve. Looping is useful to create "pulsations". 

![Envelope Looping](/Documentation/Pictures/Envelope%20Looping.png)
