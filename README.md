# Syntherrupter

Powerful polyphonic MIDI interrupter for tesla coils based on the TI Tiva TM4C1294XL microcontroller board and a Nextion resistive touch display.
You can control up to 6 different coils simultaneously. On each coil you can play over 10 notes. Probably more than you'll ever need. 

By Max Zuidberg. Credits [below](#credits).

## Index 

* [Features](#features)
* [Documentation and Getting Started](#documentation-and-getting-started)
* [Pictures](#pictures)
* [Demo Videos](#demo-videos)
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

## Documentation and Getting Started

Pretty much everything can be found in the [Wiki](/Documentation/Wiki#readme). For newcomers, there's a [Getting Started Guide](/Documentation/Wiki/Getting%20Started.md#readme). 

## Pictures

![UI Preview](/Documentation/Pictures/UI/MIDI%20Live%20Dark.png)

My own version. Left to right: On/Off switch, charge port, serial port, optical out (only one for now):
![Syntherrupter Max Back](/Documentation/Pictures/Syntherrupter_Max_Back.jpeg)

(Can be even simpler than this!)
![Syntherrupter Max Inside](/Documentation/Pictures/Syntherrupter_Max_Internal.jpeg)

## Demo Videos

Syntherrupter playing "I Want It All" - a MIDI with ~6 voices, pitch bending, sometimes very fast notes, and other effects.

[![Syntherrupter Demo - I Want It All](http://img.youtube.com/vi/H2ykCsD_b5g/0.jpg)](http://www.youtube.com/watch?v=H2ykCsD_b5g)

Here's a 6 channel demo of Syntherrupters stereo features with Thunderstruck. The only modification made to the MIDI file, were the commands that set up Syntherrupters stereo mode (documented [here](/Documentation/Wiki/Custom%20MIDI%20Commands.md)). The mapping of the notes to the coils (represented by LEDs) in done automatically.

*"I must say that the Omni-mode is what I have dreamt about for years, I am really looking forward to use that feature (and not so much that I now have to build 6 identical coils)"* ([from Mads Barnkob](https://highvoltageforum.net/index.php?topic=1020.msg8430#msg8430))

[![Syntherrupter Demo - I Want It All](http://img.youtube.com/vi/Tyts9u0le6A/0.jpg)](http://www.youtube.com/watch?v=Tyts9u0le6A)

## Credits

[Netzpfuscher and his awesome UD3](https://highvoltageforum.net/index.php?topic=188.0). Thank you for the initial help with polyphony and the awesome lightsaber idea.

[TMaxElectronics](https://tmax-electronics.de/easteregg/). Many great discussions about MIDI, Interrupters, C/C++ worst practices, and much more. And for developping a competing interrupter - keeps the development going. 

[highvoltageforum](https://highvoltageforum.net). Without those people sharing their ideas and knowledge, Syntherrupter would never be where it is now. 
