# Syntherrupter

Powerful MIDI interrupter with touchscreen for up to 6 tesla coils and 16 voices per coil. 

By Max Zuidberg. Credits [below](#credits).

## Index 

* [Features](#features)
* [Documentation and Getting Started](#documentation-and-getting-started)
* [Pictures](#pictures)
* [Demo Videos](#demo-videos)
* [PC MIDI Setup](#pc-midi-setup)
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

SimonNwardUKs version for 4 coils:
![Syntherrupter Max Back](/Documentation/Pictures/Syntherrupter_SimonNwardUK_Top_Both.jpg)

Internals of my single output version (with excessive optical receiver):
![Syntherrupter Max Inside](/Documentation/Pictures/Syntherrupter_Max_Internal.jpeg)

## Demo Videos

The following two videos demonstrate Syntherrupters capabilities but there are no arcs because I haven't had any opportunity to run my tesla coil. 

* [Queen - I Want It All](http://www.youtube.com/watch?v=H2ykCsD_b5g). A MIDI with ~6 voices, pitch bending, sometimes very fast notes, and other effects.

* [ACDC - Thunderstruck](http://www.youtube.com/watch?v=Tyts9u0le6A). This is a 6 channel demo of Syntherrupters stereo features with Thunderstruck. The only modification made to the MIDI file, were the commands that set up Syntherrupters stereo mode (documented [here](/Documentation/Wiki/Custom%20MIDI%20Commands.md)). The mapping of the notes to the coils (represented by LEDs) in done automatically.

    *"I must say that the Omni-mode is what I have dreamt about for years, I am really looking forward to use that feature (and not so much that I now have to build 6 identical coils)"* ([from Mads Barnkob](https://highvoltageforum.net/index.php?topic=1020.msg8430#msg8430))

* [ABBA - Gimme! Gimme! Gimme!](https://www.youtube.com/watch?v=6n-RvNdsbxE). Perfect demo of the [staccato envelope](/Documentation/Wiki/Envelopes.md#readme). Note how the low notes are not "on, off" but more like a quick fade in and fade out with slight reverb. That's the envelope. Compare it with the videos below, which do not use this feature

Then there are a few videos from other people that build their own Syntherrupter:
* [Video (performed by Georgios)](https://www.youtube.com/watch?v=1vgiw4VHPKQ). Fast MIDI with 2 tesla coils and up to 4 notes per coil
* [O-Zone - Dragostea Din Tei (performed by futurist)](https://www.youtube.com/watch?v=86U6sI6FZ6c). 1 Coil, two notes, nonetheless a good MIDI.
* [Fonsi - Despacito and much more (performed by futurist)](https://www.youtube.com/playlist?list=PLlkH_ZpBGiexB1ahBjtajvHqJAfMIxrB1). Playlist with a lot of popular songs. Usually only 2 notes at most, played by 2 coils. 

## PC MIDI Setup

To open and play MIDI files I use the free software [SynthFont 1](http://www.synthfont.com/). You can select a different output for each track which we can use to send some tracks to the interrupter. You might want to change the instrument of these tracks to use Syntherrupters [envelopes](/Documentation/Wiki/Envelopes.md#readme). For good sound quality download the [SGMv2.01-Sal-Guitar-Bass-v1.3](https://sites.google.com/site/soundfonts4u) soundfont).

While SynthFont allows you to send MIDI data directly to a serial port and thus to Syntherrupter, I do recommend another solution. In my experience it is easier to use, more reliable and works with any MIDI software you like. All you need are two little tools that are very easy to use: [Hairless MIDI<->Serial](https://projectgus.github.io/hairless-midiserial/) and [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html). When you start loopMIDI it creates a virtual MIDI device that you can select in any MIDI software as output (or input btw). And hairless MIDI<->Serial makes the bridge between this virtual MIDI device and the serial COM port.
This sounds complicated but since both programs remember all settings, you only have to start them - no other click needed. The setup has proven its reliability and usability many times and for many hours. 

## Credits

[Netzpfuscher and his awesome UD3](https://highvoltageforum.net/index.php?topic=188.0). Thank you for the initial help with polyphony and the awesome lightsaber idea.

[TMaxElectronics](https://tmax-electronics.de/easteregg/). Many great discussions about MIDI, Interrupters, C/C++ worst practices, and much more. And for developing a competing interrupter - keeps the development going. 

[highvoltageforum](https://highvoltageforum.net). Without those people sharing their ideas and knowledge, Syntherrupter would never be where it is now. 
