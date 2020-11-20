# Minimum Viable Setup

[Back to the Getting Started Guide](Getting%20Started.md#readme)

The minimum viable setup is the bare minimum you need to even run the Syntherrupter firmware. It won't be ready to use yet but it will allow you to flash firmwares and get to know the user interface. 

## Index

* [Index](#index)
* [Firmware](#firmware)
* [Tiva Microcontroller](#tiva-microcontroller)
* [Display](#display)
	* [Introduction](#introduction)
	* [Suitable Models](#suitable-models)
	* [Without a Display](#without-a-display)
	* [Wiring](#wiring)

## Firmware

You'll always need a Syntherrupter firmware, preferrably the latest release. You find all releases here: https://github.com/MMMZZZZ/Syntherrupter/releases/

Read the release notes and download the attached ZIP file. It includes all the firmware binaries you'll need. 

Details about how to flash those firmwares are described on a separate page: [Firmware Flashing](Firmware%20Flashing.md#readme). 

## Tiva Microcontroller
The heart of Syntherrupter is the TM4C1294XL LaunchPad from TI. The Microcontroller is part of their *Tiva* family (Cortex M4F microcontrollers), hence I'll call it *Tiva* in the following.

This dev board can be bought [directly from Texas Instruments](https://www.ti.com/tool/EK-TM4C1294XL#order-start-development) for 20$, or from distributors like Mouser.

Everything else will connect to this board. How? Where? What pins? Beside the information given here you should check the *[Wiring and Schematics](/Documentation/Wiring%20and%20Schematics)* folder which includes schematics and pinouts. 

## Display

### Introduction
Syntherrupters UI is designed for a special touch display called "Nextion". They have their own microcontroller that runs the whole UI which makes development *a lot* easier. Assembly gets easier, too. No 40 pin flat-flex cable, only 5V supply and two data pins. 

There are multiple display versions that can do the job. If you don't care and simply want to go with the recommended display from the recommended shop, click [here](https://www.itead.cc/nextion-nx8048k050.html) and you're good.

Btw. since they are resistive touch screens, they aren't affected by EMI like the capacitive screen of your smartpone. 

You don't need a standalone interrupter? Well, you can run the user interface on the PC, too. In that case you don't need a display at all ([see below](#without-a-display)).

### Suitable Models
Nextions ease of use comes at a cost. Litterally. These displays are rather expensive, starting at 65$ for the 5" version. Starting? Yes, there are actually 4 displays that are suitable for Syntherrupter:

* `NX8048T050_011R`: Nextion 5" Basic series ([Official shop: 65$](https://www.itead.cc/nextion-nx8048t050.html))
* `NX8048T070_011R`: Nextion 7" Basic series ([Official shop: 75$](https://www.itead.cc/nextion-nx8048t070.html))
* `NX8048K050_011R`: Nextion 5" Enhanced series ([Official shop: 72$](https://www.itead.cc/nextion-nx8048k050.html))
* `NX8048K070_011R`: Nextion 7" Enhanced series ([Official shop: 83$](https://www.itead.cc/nextion-nx8048k070.html))

I recommend buying them from the official shop. 

The R at the end is important! It indicates that its the version with *resistive* touch. There are also versions with *capacitive* touch, marked by a `C` and versions with *no touch*, marked with an `N`. 

Among other things that are useless for Syntherrupter the enhanced series offers more than twice the processing power. This allows for a more responsive UI. In addition not every store has all of those displays. Mouser offers only the most expensive one from the list. 

Plot twist: Nextion is actually just a rebranding. The displays originally come from the chinese manufacturer TJC. Buying directly from those is 30-50% cheaper but they only sell within China so you have to use an import service like Superbuy. Still cheaper than buying a Nextion but also slower. 

The TJC displays have the same model numbers except that they start with `TJC` instead of `NX`:

* `TJC8048T050_011R`: TJC 5" Basic series
* `TJC8048T070_011R`: TJC 7" Basic series
* `TJC8048K050_011R`: TJC 5" Enhanced series
* `TJC8048K070_011R`: TJC 7" Enhanced series

To keep things simple I won't write "Nextion or TJC display" every time but simply call it "Nextion". 

**Warning!** You can find the displays on ebay, too. However, in most cases you'll get a TJC display even though it says Nextion. For Syntherrupter this doesn't make a difference but if you ever want to do something with them on your own, be careful about this or you will run into problems. 

### Without a display

The user interface can be run on a Windows PC. This requires an additional Serial to USB converter that connects to the same Pins as the Nextion display would ([see below](#wiring)).

* Download, install and open the Nextion Editor. For firmwares up to  v4.1.0-beta.6 you need Nextion Editor v1.61.1 (Download as [ZIP (portable)](https://nextion.tech/download/nextion-setup-v1-61-1.zip) or [Installer](https://nextion.tech/download/nextion-setup-v1-61-1.exe)). Newer releases will be created with the current version of their Editor. Unfortunately each of their updates breaks compatibility... I'll keep this section updated.
* Click "Debug" on the top right. A file dialog opens. Select the *Syntherrupter_Nextion_NX8048T050.tft* file from the release zip file. 
* A new window opens. At the bottom left select "User MCU Input" and then the COM port of the Serial to USB converter. Baud rate is 115200baud/s.
* You may need to reset the Tiva. 
* You should see the Syntherrupter logo. Since its a touch screen firmware, you can only use your mouse; no keyboard. 

### Wiring

Tiva pins:
* PA4 connects to Nextions RX pin (yellow)
* PA5 connects to Nextions TX pin (blue)

Both Tiva pins are 3.3V logic pins, capable of 2mA current. They are 5V tolerant.

The Nextion display requires regulated 5V at 500mA. If possible you should not power the display through the Tiva LaunchPad. The PWM modulated backlight causes a rather strong ripple that may cause issues (even though I never had any).
