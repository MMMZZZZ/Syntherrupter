# Syntherrupter

Powerful polyphonic MIDI interrupter for tesla coils based on the TI Tiva TM4C1294XL microcontroller board and a Nextion resistive touch display.
You can control up to 6 different coils simultaneously. On each coil you can play over 10 notes. Probably more than you'll ever need. 

By Max Zuidberg. Credits [below](#credits).

## Index 

* [Features](#features)
* [Pictures](#pictures)
* [Demo Videos](#demo-videos)
* [Documentation and Getting Started](#documentation-and-getting-started)
* [Credits](#credits)

## Features

* A unique and truly awesome name: Syntherrupter. ;D 
* **Easy to build**. Except for the optical transmitters the other parts are "ready to use" modules that can be connected without soldering or custom PCBs.
* **Over 10 voice polyphony** including effects like pitch bend, modulation and different "instruments". Yes, you can - within limits - change how your coil sounds.
* Control up to **6 independant tesla coils** with only one interrupter. And yes, each output can play different notes. Simply select which output should listen to which MIDI channel.
* Set and change hard limits for each coil. They will be stored even when powered off and assure that you don't fry your tesla coil no matter how crazy the MIDI file is.
* Advanced stereo features. Notes and MIDI channels can seamlessly change between multiple coils, creating **fascinating visual effects**.
* **Lightsabers**! An ESP8266, an IMU and a battery form a lightsaber that connects to Syntherrupter and allows "lightsaber-effects" to be played on your tesla coils! (credits: Netzpfuscher)
* Different users with different limits. This is useful if you want to rent the coil to someone else who does not know the coils (thermal?) limits as well as you do.
* Sounds boring, but for me it belongs to an interrupter as well: **Normal interrupter mode** where you can control the ontime, BPS and duty.
* **[Documentation/Wiki](/Documentation/Wiki#readme)**

## Pictures

![UI Preview](/Documentation/Pictures/UI/MIDI%20Live%20Dark.png)

My own version. Left to right: On/Off switch, charge port, serial port, optical out (only one for now):
![Syntherrupter Max Back](/Documentation/Pictures/Syntherrupter_Max_Back.jpeg)

(Can be even simpler than this!)
![Syntherrupter Max Inside](/Documentation/Pictures/Syntherrupter_Max_Internal.jpeg)

## Demo Videos

Syntherrupter playing "I Want It All" - a MIDI with ~6 voices, pitch bending, sometimes very fast notes, and other effects.

[![Syntherrupter Demo - I Want It All](http://img.youtube.com/vi/H2ykCsD_b5g/0.jpg)](http://www.youtube.com/watch?v=H2ykCsD_b5g)

Here's a demo of Syntherrupters stereo features with Thunderstruck. The only modification made to the MIDI file, were the commands that set up Syntherrupters stereo mode (documented [here](/Documentation/Wiki/Custom%20MIDI%20Commands.md)). The mapping of the notes to the coils (represented by LEDs) in done automatically.

*"I must say that the Omni-mode is what I have dreamt about for years, I am really looking forward to use that feature (and not so much that I now have to build 6 identical coils)"* ([from Mads Barnkob](https://highvoltageforum.net/index.php?topic=1020.msg8430#msg8430))

[![Syntherrupter Demo - I Want It All](http://img.youtube.com/vi/Tyts9u0le6A/0.jpg)](http://www.youtube.com/watch?v=Tyts9u0le6A)

## Documentation and Getting Started

Check the [Wiki](/Documentation/Wiki#readme). It's far from being complete, but the most important stuff - like a Getting Started Guide - is there.

### Envelope Sounds

The sound can be selected by the MIDI command "Program Change". In your MIDI software you can select it by changing the instrument of the channel. 

The programs 1-63 can be modified by the user using the [Envelope Editor](/Documentation/Wiki/UI/Envelope.md#readme). The programs 20-39 are stored in EEPROM. Unless modified by the user, they are simply constant ontime. All other programs have the following characteristics at startup:

* Program 0 and all unlisted: No envelope. Constant ontime (except for other effects like modulation).
* Program 1: Roughly like a piano. Attack peaks to 1 (= not exceeding the given ontime)
* Program 2: Sloooow rise, slow fall. Good for soft background, but too slow for shorter notes. Attack peaks to 1 (= not exceeding the given ontime)
* Program 3: Like program 1, but with a small step, to make shorter notes more audible. Attack peaks to 1 (= not exceeding the given ontime)
* Program 4: Twice as fast as program 1. Attack peaks to 1 (= not exceeding the given ontime)
* Program 5: Forced Staccato. All notes are always short. Attack peaks to 1 (= not exceeding the given ontime)
* Program 6: Forced Legato. All notes are hold for quite some time. Attack peaks to 1 (= not exceeding the given ontime)
* Program 7: Like program 2, but with a faster release. Attack peaks to 1 (= not exceeding the given ontime)
* Program 8: Roughly like a piano. Attack peaks to 2 (= doublingt he given ontime for a few miliseconds)
* Program 9: Forced Staccato with slight sustain. Attack peaks to 3 (= tripling the given ontime for a few miliseconds)
* Program 11-19: Same as programs 1-9 but with exponential instead of linear curves between the points. No other changes made, so not that special yet. 

If you wonder why you would want to exceed the given ontime, it is an "efficient" way to get louder notes even with high voice count without tripping your circuit breaker. Since the ontime drops pretty fast after the attack you can consider the ontime on the display more like an average ontime. Note that all these envelope settings do not allow to exceed your [coil limits](/Documentation/Wiki/UI/Coil%20Limits.md#readme). 

### Other documentation

The forum threads (especially the english one) contain many posts and videos explaining and demonstrating the features of v2.x.x, v3.x.x and v4.x.x. Anything that's not yet been described in the wiki can be found there.

English: https://highvoltageforum.net/index.php?topic=1020.0

German: http://forum.mosfetkiller.de/viewtopic.php?f=9&t=64458


## Credits

[Netzpfuscher and his awesome UD3](https://highvoltageforum.net/index.php?topic=188.0). Thank you for the initial help with polyphony and the awesome lightsaber idea.

[TMaxElectronics](https://tmax-electronics.de/easteregg/). Many great discussions about MIDI, Interrupters, C/C++ worst practices, and much more. And for developping a competing interrupter - keeps the development going. 

[highvoltageforum](https://highvoltageforum.net). Without those people sharing their ideas and knowledge, Syntherrupter would never be where it is now. 
