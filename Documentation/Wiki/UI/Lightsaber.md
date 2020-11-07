# Lightsaber Mode

[Back to the User Interface Reference page](README.md#readme)

Description

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)

## What you see

![Lightsaber Mode](/Documentation/Pictures/UI/Lightsaber.png)

## What you get

### Ontime

Using the slider you can vary the ontime within the [user's limits](User%20Settings.md#readme). This is the maximum ontime a lightsaber effect will generate. Usually the ontime is much lower.

### Lightsabers

Select which lightsabers play on the current outputs. This is similar to the [MIDI channel selection](Coil%20Settings.md#readme) in MIDI Live mode. 

Keeping any of these buttons pressed, assigns that ID to the lightsaber that's currently connected. More details on this can be found in the [Lightsaber Setup](/Documentation/Wiki/Lightsaber%20ESP8266%20Setup.md#lightsaber-id-setting) article.

### Apply to Outputs and MIDI Coil Settings

Whenever you change a setting it will affect the outputs selected here. You'll note that the values below the buttons change to your current settings. More details about this is explained on the [Simple mode](Simple.md#apply-to-outputs) page. 

Unlike the information displayed in [Simple mode](Simple.md#apply-to-outputs), the displayed ontime can exceed the [Coil Limits](Coil%20Limits.md#readme). That is because it represents the maximum within which the lightsaber effect operates. Most of the time you'll see ontimes smaller than these on the output. As a reminder, the actual safety mechanisms are applied as a last step to the output signal. They work independently of what you see on the display.

### Applying Manually, On Release or Immediately

Works in the same way as described here: [Simple Mode](Simple.md#applying-manually-on-release-or-immediately).
