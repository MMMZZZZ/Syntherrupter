# Getting Started with Syntherrupter

[Back to the wiki main page](README.md)

## Index
* [Introduction](#introduction)
* [Hardware](#hardware)
	* [Minimum Viable Setup](#minimum-viable-setup)
	* [Options](#options)
* [Firmware](#firmware)
* [User Interface](#user-interface)
	* [First Start](#first-start)
	* [Login Page](#login-page)
	* [Main Menu](#main-menu)
	* [Simple Mode](#simple-mode)
	* [MIDI Live Mode and Lightsaber Mode](#midi-live-mode-and-lightsaber-mode)
	* [Settings](#settings)
* [Done!](#done)

## Introduction

So you want to build your own Syntherrupter? Great! This page will guide you. First you'll learn about what you need to build one, then how you get it ready to use, and finally we'll take a quick tour through the user interface. 



## Hardware

There's no finished PCB which you could buy and populate. Since Syntherrupter is based on a dev board many things can be connected via jumper wires. However, depending on which features you want to include, you'll probably want to use a breadboard PCB or design your own PCB. 

### Minimum Viable Setup

No matter what features you want to (not) have this is what you'll always need. The minimum viable setup doesn't require any soldering or additional components. 

Link to the page: [Minimum Viable Setup](Minimum%20Viable%20Setup.md)

### Options

Once you have the minimum viable setup, you can extend it by any combination of the following options. Each option is described on its own page.

* [Optical transmitters for up to 6 independant coils](HW/Optical%20Transmitters.md)
* [Shielded encasing]((HW/Casing.md)
* [Powerbank for standalone operation](HW/Power.md)
* [MIDI Input jack](HW/MIDI%20In.md)
* [Lightsabers](HW/Lightsabers.md)

## Firmware

Once the hardware is done, you can flash the Syntherrupter firmware. Step-by-step guide over here: [Firmware Flashing](Firmware%20Flashing.md). 

*Note: actually, you could already flash the firmware once you have the minimum viable setup, but I assume most people want to get the hardware finished before messing with software.*

## User Interface

Firmwares flashed? Then let's get this party started!

### First Start
The first time Syntherrupter is powered up, you'll see a warning. Since nobody ever used this thing, its EEPROM is still empty and contains no valid settings. This warning will stay until you actually open and close the settings. 

Let's ignore this warning for a second and take a look at the UI first. Note that at this point all safety limits are at 0. This means you won't get any output signals until you change them in the settings. We'll get to that later.

Hit any part of the screen to continue. 

### Login Page

Syntherrupter supports three different users with different rights. By default you'll automatically log in as "Master Yoda", the user with all rights (think of it as the "Admin" or "root user").

More details about the different users can be found here: [User Management](User%20Management.md)

### Main Menu

The Main Menu links everything together. It's the "root" of the user interface. 

It contains 6 buttons. 
* At top you have the three modes of operation of Syntherrupter: Simple, MIDI Live and Lightsaber. Clicking on them brings you the settings for that mode. 
* Then we have Help and Info, which is kind of a work in progress. I plan to put more info in there but for it only contains a Credits section and some info you'd probably not find by "trial&error". Don't worry, we'll get to it.
* Next one in the row is Settings. This is where you configure things like Username/password, screen brightness, safety limits and many other things
* Last one is Switch User. It brings you back to the login page. At this point I should probably tell you, that the auto login password is `0`. 

### Simple Mode

Let's check Simple mode first. This is a good place to learn about some basic usage concepts of Syntherrupter. 

#### The Sliders

The page contains three sliders, for ontime, BPS/frequency and duty cycle. If you wonder why you need three, you're kinda right, three is redundant. This has two reasons: sometimes you want to control the "power" via the ontime, and sometimes by the duty cycle. The second reason is that no matter how you like to control the signal, you probably always want to know both values. First lesson: Syntherrupter aims for flexibility.

When you play with the sliders, you'll see that all values update automatically. If you increase the ontime, the duty increases, too. Where it gets interesting is when you change the BPS. Should it keep the ontime or keep the duty setting? Answer: It keeps whatever you edited last. Try it out, change the ontime, then the BPS. Now set the duty cycle and change the BPS again. Second lesson: Syntherrupter tries to be smart. 

#### Selecting the outputs

If you have more than one output, you'll a button for each output below the sliders. An "empty" button means that this output will not be affected by the next change you make. A "filled" button on the other hand means this output *will* be set to whatever you enter with the sliders.

Under each button you'll find what values are currently active on this output. Imagine you have six coils and you set each coil to different values - you wouldn't remember the values of the first one when you're done with the last one. Okay let's be fair, maybe you would but I wouldn't for sure.

Note that for now the ontime stays at zero because the coil limits don't allow any higher value yet. As I said we'll check this later. 

#### Apply manually or automatically

Now that you know *where* your settings will be applied, let's talk about *when* this happens

At the bottom you'll see a button that says *On Release*. This is when the values you enter will be applied. The moment you release any of the three sliders at the top, the settings will be applied to all the outputs you have currently selected. 

If you press that *On Release* button, it will change to *Manually* and another button appears: *Apply Now*. Now you can make as many changes to the sliders you want. Only when you click the *Apply Now* button they'll affect the outputs. Useful if you want to change f.ex. both ontime and BPS. 

Pressing the *Manually* button "again" and it changes to *Immediately*. This means that the outputs will already update while you move the sliders. 

#### Emergency stop

**Important**: If your coil is burning, cut the power and/or unplug the fibre cables. Don't rely on any software features.

Assuming a less urgent emergency (whatever that means), hit the empty background, f.ex. right of the return button. You'll see that all outputs jump to zero ontime and zero duty. 

Change at least one output back to anything non-zero and press the title. The same thing happens. 

To sum up: anything that's not a button, not a slider and in general not interactive will disable the outputs. This works on all pages. 

#### Running Modes in Parallel

By now you know pretty much all there's to know for the Simple mode. Let's return to the main menu. But wait - what's that? Why does that return button have a double outline?

A double outline indicates that the button has not only a "normal" function (f.ex. returning to the main menu) but an additional function when you keep it pressed. 

In the case of the return button a simple push stops Simple mode and brings you back to the main menu. If you keep it pressed, it will still bring you back to the main menu, but simple mode will continue to run in the background. Admitted this feature makes not a lot of sense for Simple mode, but it does make a ton of sense for MIDI Live mode and Lightsaber mode.

If you look at the bottom right, you'll see the three modes listed. They indicate what mode is currently active. When you entered Simple mode the according field got highlighted. If you keep the return button pressed, you'll see that it stays highlighted even though you're back to the main menu. 

At this point you may think again about the emergency stop. Imagine how much time it would take to cycle through all modes and stop each one of them. The emergency stop disables the outputs entirely, no matter what mode is active. 

### MIDI Live Mode and Lightsaber Mode

They deserve their own page. 

### Settings

Let's leave the operation modes for now and look at the settings. 

There are 6 settings pages:
* General
* Envelope
* Users
* Coil Limits
* Nextion<->USB
* ESP<->USB

#### Coil Limits

Let's look at the Coil Limits because they cover the most important safety feature of Syntherrupter. These settings apply equally to all modes. In fact, all modes sit on top of what I call the "Core" of Syntherrupter. That's the code that actually generates all the output signals. Each mode can add tones to any outputs. The Core then checks all tones of all modes together against all the limits and generates an output signal that exceeds none of them. 

The following limits can be set:
* *max. Ontime* (0us-41ms) specifies what's the longest ontime. 
* *min Offtime* (0us-1270us) specifies the minimum time between two succeeding ontimes. This is necessary because the primary current needs time to drop back to zero after an ontime. Since all tesla coil drivers I'm aware of assume the primary current to be zero at the beginning of the ontime, a too small min. Offtime would lead to hardswitching. 
* *max. Duty* (0%-51%) highest duty cycle of the output signal. This is mainly useful to prevent any components from overheating. It is calculated by adding the duty cycle of every single tone. 
* *max. MIDI Voices* (1-16) limits the number of simultaneous voices in MIDI Live mode. This does not affect the other modes when running multiple modes in parallel. If you set the limit to 6, you will not hear more than 4 simultaneous notes in your MIDI playback. However you can still have 4 Lightsabers playing in addition to the 6 MIDI voices on the same output. The reason for the limit affecting only the MIDI Live mode is that you can easily control the number of lightsabers that play. No need somehow limit that. 

You can modify any of the values by tipping on it and typing a new value using the keypad at the bottom right. The settings will be applied when you hit the return button. 

#### User Settings

I'll quickly talk about this one because there're a few pitfalls.

Similar to the Coil Limit page you get a table with the different settings for each user. The leftmost user has the least rights, the rightmost user is the "Admin"/"root user". 

First pitfall, if you want to change the user passwords, remember that on the login screen you only have a keypad! Don't enter anything else than digits! Password length can be anything from 1-16. 

Out of the box the Admin password is `0`. This specific password tells Syntherrupter to log in automatically into this account. Of course, you can change this password and/or set the password of another user to `0`, which causes Syntherrupter to auto login into that account. 

Next pitfall are the user limits. They are independant of the coil limits and they only affect the ranges of the sliders. F.ex. if the user duty limit is set to 5% you can't set the per-note duty higher than 5%. However, you can still get a 10% output signal when playing multiple notes simultaneously. This of course assumes that your coil limits allow such a signal. 

Before you're getting crazy, no you can't edit the ontime and duty settings of the "Admin". They are automatically set to the highest of your coil limits. By definition the Admin has full rights, hence a lower value makes no sense. And by definition of the coi limits a higher value makes no sense either. 

#### Applying and Storing

Most settings are applied when leaving the according settings page. However at this point they're not yet written to the EEPROM (and thus remembered in case of a power cycle). This only happens when you return to the main menu. Hence that button says *Save* instead of *Return*. 

If you power cycle Syntherrupter now, you won't get the warning anymore. The EEPROM now contains a valid config. 

## Done!

You've learned the basics about Syntherrupter. There's a lot more to learn and unfortunately not everything is obvious. However, you know the most important concepts and should be able to explore many if Syntherrupters features on your own. 

Don't be afraid of playing around; remeber, the coil limits protect your coil from oopsies.
