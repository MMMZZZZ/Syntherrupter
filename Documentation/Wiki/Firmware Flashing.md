# Syntherrupter Firmware Flashing

[Back to the wiki main page](README.md#readme)

This page covers how to flash a (new) firmware on your Syntherrupter.

## Index
* [Index](#index)
* [Note about beta versions](#note-about-beta-versions)
* [Flashing Order and Recovery](#flashing-order-and-recovery)
* [Tiva Flashing](#tiva-flashing)
* [Nextion Flashing over USB/Serial](#nextion-flashing-over-usbserial)
* [Nextion Flashing with a microSD card](#nextion-flashing-with-a-microsd-card)

## Note about beta versions
Beta versions usually include new features that do not touch the core of Syntherrupter. This means it's *highly unlikely* that the update breaks the core's safety features, namely the specified maximum ontime, minimum offtime and maximum duty. However, you should not expect any part of the GUI or any of Syntherrupter's modes to be free of bugs. 

## Flashing Order and Recovery
Firmware updates always consist of a new Nextion and a new Tiva firmware. Unless the release notes say something different, you should update the Nextion firmware first. The reason is that you can always flash the Tiva microcontroller but in order to flash Nextion you need a working Syntherrupter that allows you to enter Nextion<->USB passthrough mode.

Usually if a Nextion firmware update goes wrong, the Tiva microcontroller will automatically enter Nextion<->USB passthrough mode, allowing you to reflash the firmware. However, if you flashed f.ex. the wrong version it can happen that this check is successful but Syntherrupter is still unuasble. In this case you can either [update Nextion using a microSD card](#nextion-flashing-with-a-microsd-card), or disconnect the Nextion screen when powering up Syntherrupter. Wait 5 seconds, and reconnect. This makes sure that the Tiva firmware entered Nextion<->USB passthrough mode and you can [flash the Nextion firmware over USB](#nextion-flashing-over-usbserial) again. 

## Tiva Flashing

### Requirements
* Windows. The tools used are not available for other operating systems.
* [Latest Syntherrupter release](https://github.com/MMMZZZZ/Syntherrupter/releases/)
* [Drivers](/Utilities%20and%20Drivers/Tiva%20USB%20Drivers%20(Stellaris%20ICDI%20Drivers).zip?raw=true)
* [LM Flash Programmer](/Utilities%20and%20Drivers/LMFlashProgrammer_1613.msi?raw=true)

### Steps for Driver Installation
* Extract the *stellaris_icdi_drivers* folder from the drivers zip file.
* Connect Tiva LaunchPad to the PC via its DEBUG USB port. 
* Open the Device Manager. One or more unknown "ICDI" devices will be listed. 
* Right click on one of them
	* Choose update the drivers,
	* Search on the PC, 
	* Select the unzipped "stellaris_icdi_drivers". 
	* Repeat for all other unknown "ICDI" devices.

### Steps for Flashing
* Extract the Tiva firmware from the Syntherrupter release zip file. Based on the number of optic transmitters you have, select the right version. The firmware file (for X transmitters) is called "Syntherrupter_Tiva_X_Coils.bin".
* Install and open LM Flash Programmer. 
* On the tab *Configuration*, select *TM4C1294XL LaunchPad* from the *Quick Set* drop down list. 
* Switch to the tab "Program" and select the extracted .bin file.
* Select the following options: 
	* *Erase Entire Flash*
	* *Verify After Program*
	* *Reset MCU After Program*
* Click on *Program* to flash the microcontroller.

## Nextion Flashing over USB/Serial
You can upload a firmware to the Nextion screen using its serial connection. This is very convenient because you don't have to access the microSD card slot on the screen itself. 

Note: While the Nextion screen is configured to communicate at 115200baud/s, the Nextion upload protocol allows to switch to a different baud rate during the firmware upload. The highest possible baud rate is 512000baud/s. This speeds up the upload considerably, "only" taking about 1 minute.

### Requirements
* Windows. The tools used are not available for other operating systems.
* [Tiva Firmware](#tiva-flashing) up and running. 
* [Latest Syntherrupter release](https://github.com/MMMZZZZ/Syntherrupter/releases/)
* [TFTFileDownload](/Utilities%20and%20Drivers/TFTFileDownload.exe?raw=true)

### Steps
* Check the exact model number of your touch screen. 
* Extract the Nextion firmware for your model number from the Syntherrupter release zip file. The firmware file is called "Syntherrupter_Nextion_MODEL.tft". 
* Connect Syntherrupter to your computer and choose *Nextion<->USB* in Syntherrupters *Settings* menu. If your Nextion firmware is not working, the Tiva Firmware will automatically enter this mode after 3 seconds. 
* Open the TFTFileDownload tool and select the COM port. Baud rate should be 512000baud/s. In case you get timeout or other errors, try a lower baud rate. If even at 115200baud/s you can't upload the firmware, there's something wrong.
* Click on the *Down* button to start flashing. 
* Once the upload is done, the screen will reset and stay black until you power cycle Syntherrupter. 

## Nextion Flashing with a microSD card
In case the serial upload does not work or is too slow for you, you can try uploading the firmware using a microSD card. This update method is also useful if you have no access to a Windows PC. 

### Requirements
* [Latest Syntherrupter release](https://github.com/MMMZZZZ/Syntherrupter/releases/)
* microSD card. Note: some microSD cards do not work with the Nextion screen. I cannot tell you if your card will work or not. 

### Steps
* Make sure the microSD card is formatted as FAT32 and empty.
* Check the exact model number of your touch screen. 
* Copy the Nextion firmware for your model number from the Syntherrupter release zip file to the microSD card. The firmware file is called "Syntherrupter_Nextion_MODEL.tft". 
* Insert the microSD card into your screen and power it on. The upload starts. 
* It is possible that the upload fails because you actually got a TJC screen, and not a Nextion screen. Try again with the *TJC8048...* firmware instead of the *NX8048...* file. 
* Once the upload has completed, power off your screen and remove the microSD card.

