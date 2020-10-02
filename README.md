# Syntherrupter
Powerful polyphonic MIDI Interrupter with GUI for Tesla Coils based on the TI Tiva TM4C1294XL microcontroller board and a Nextion resistive Touch Display.
You can control up to 6 different coils simultaneously. On each coil you can play up to 16 notes - making a total of 96 voices. Probably more than you'll ever need.

*By Max Zuidberg. Credits below.*

## Demo Videos
Syntherrupter playing "I Want It All" - a MIDI with ~6 voices, pitch bending, sometimes very fast notes, and other effects.

[![Syntherrupter Demo - I Want It All](http://img.youtube.com/vi/H2ykCsD_b5g/0.jpg)](http://www.youtube.com/watch?v=H2ykCsD_b5g)

Here's a demo of Syntherrupters stereo features with Thunderstruck. The only modification made to the MIDI file, were the commands that set up Syntherrupters stereo mode (documented [here](/Documentation/Wiki/Custom%20MIDI%20Commands.md)). The mapping of the notes to the coils (represented by LEDs) in done automatically.

[![Syntherrupter Demo - I Want It All](http://img.youtube.com/vi/Tyts9u0le6A/0.jpg)](http://www.youtube.com/watch?v=Tyts9u0le6A)

## Features
* A unique and truly awesome name: Syntherrupter. ;D 
* **Easy to build**. Except for the optical transmitters the other parts are "ready to use" modules that can be connected without soldering or custom PCBs.
* Up to **16 voice polyphony** including effects like pitch bend, modulation and different "instruments". Yes, you can - within limits - change how your coil sounds.
* Control up to **6 independant tesla coils** with only one interrupter. And yes, each output can play different notes. Simply select which output should listen to which MIDI channel
* Set and change hard limits for each coil. They will be stored even when powered off and assure that you don't fry your tesla coil no matter how crazy the MIDI file is.
* Advanced stereo features. Voices can seamlessly change between multiple coils, creating **fascinating visual effects**.
* **Lightsabers**! An ESP8266, an IMU and a battery form a lightsaber that connects to Syntherrupter and allows "lightsaber-effects" to be played on your tesla coils! (credits: Netzpfuscher)
* Different users with different limits. This is useful if you want to rent the coil to someone else who does not know the coils (thermal?) limits as well as you do.
* Sounds boring, but for me it belongs to an interrupter as well: **Normal interrupter mode** where you can control the ontime, BPS and duty.

## Download and Installation
Have a look at the [releases page](https://github.com/MMMZZZZ/Syntherrupter/releases) and the forum links below.

## Documentation
Check the [Wiki](/Documentation/Wiki). It's far from being complete, but the most important stuff is there.

### ADSR Sounds
The sound can be selected by the MIDI command "Program Change". In your MIDI software you can select it by changing the instrument of the channel. 

The programs 1-63 can be modified by the user using the ADSR Editor. The programs 20-39 are stored in EEPROM. Unless modified by the user, they are simply constant ontime. All other programs have the following characteristics at startup:

* Program 0 and all unlisted: No ADSR. Constant ontime (except for other effects like modulation).
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

If you wonder why you would want to exceed the given ontime, it is an "efficient" way to get louder notes even with high voice count without tripping your circuit breaker. Since the ontime drops pretty fast after the attack you can consider the ontime on the display more like an average ontime. Note that all these ADSR settings do not allow to exceed your coil limits (->Settings->Coil Setttings). 

### Communication Pins

#### For the Nextion touch screen
* PA4: RX
* PA5: TX
#### For MIDI data at 31250baud/s
* PC4: RX
* *PC5: TX (unused)*
#### For MIDI data at 115200baud/s
These Pins are connected to the on-board USB to serial converter.
* *A0: RX*
* *A1: TX*
#### For Lightsaber Receiving ESP8266
* PA6: RX
* PA7: TX
### Output pins
The following pins supply the interrupt signal for the given coil. Signal is active high, 3.3V, max. 12mA.
* Coil 1: PD0
* Coil 2: PD2
* Coil 3: PM0
* Coil 4: PM2
* Coil 5: PM4
* Coil 6: PM6
### Other documentation
Building and using one is much easier than it looks. The forum threads (especially the english one) contain many posts and videos explaining and demonstrating the features of v2.x.x, v3.x.x and v4.x.x. Anything that's not yet been described here can be found there.

English: https://highvoltageforum.net/index.php?topic=1020.0

German: http://forum.mosfetkiller.de/viewtopic.php?f=9&t=64458


### Credits
[Netzpfuscher and his awesome UD3](https://highvoltageforum.net/index.php?topic=188.0). Thank you for the help with polyphony and the awesome lightsaber idea.

[TMaxElectronics](https://tmax-electronics.de/easteregg/). Many great discussions about MIDI, Interrupters, C/C++ bad practices, and much more. And for developping a competing Interrupter - keeps the development going. 

[highvoltageforum](https://highvoltageforum.net). Without those people sharing their ideas and knowledge, Syntherrupter would never be where it is now. 
