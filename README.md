# Syntherrupter
Powerful polyphonic MIDI Interrupter with GUI for Tesla Coils based on the TI Tiva TM4C1294XL microcontroller board and a Nextion resistive Touch Display.
You can control up to 6 different coils simultaneously. On each coil you can play up to 16 notes - making a total of 96 voices. Probably more than you'll ever need.

*By Max Zuidberg. Credits below.*

## Download and Installation
Have a look at the [releases page](https://github.com/MMMZZZZ/Syntherrupter/releases) and the forum links below.

## Documentation

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
### Output pins
The following pins supply the interrupt signal for the given coil. Signal is active high, 3.3V, max. 12mA.
* Coil 1: PD0
* Coil 2: PD2
* Coil 3: PM0
* Coil 4: PM2
* Coil 5: PM4
* Coil 6: PM6
### ADSR Sounds
The sound can be selected by the MIDI command "Program Change". In your MIDI software you can select it by changing the instrument of the channel. At the time of writing (release v4.0.0) there are the following sounds available:
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

### Other documentation
Building and using one is much easier than it looks. The forum threads (especially the english one) contain many posts and videos explaining and demonstrating the features of v2.x.x, v3.x.x and v4.x.x. Until I find the time to write a complete wiki, that's pretty much the best documentation you'll find. 

English: https://highvoltageforum.net/index.php?topic=1020.0

German: http://forum.mosfetkiller.de/viewtopic.php?f=9&t=64458

### Credits
[Netzpfuscher and his awesome UD3](https://highvoltageforum.net/index.php?topic=188.0)

[TMaxElectronics for many interesting discussions](https://tmax-electronics.de/easteregg/)

[highvoltageforum for great ideas and support](https://highvoltageforum.net)
