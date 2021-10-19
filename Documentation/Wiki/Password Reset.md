# Password Reset

Forgot your password or need to reset it? Here's how you do it.

## Firmware version 4.2.x

Use the [Sysex Commands](Custom%20MIDI%20Commands.md#system-exclusive-messages-sysex) to set a new password. 


## Firmware version 4.1.x

1. Download the [Tiva Password Recovery Firmware](https://github.com/MMMZZZZ/Syntherrupter/blob/Password-Recovery/Syntherrupter_Tiva-PW_REC.bin?raw=true).
1. Flash the firmware [like a normal update](Firmware%20Flashing.md#tiva-flashing).
1. Go to the settings and enter a [new password](/Documentation/Wiki/UI/Users.md#password).
1. Save the new password by [returning to the main menu](/Documentation/Wiki/UI/Settings.md#savereturn-to-main-menu). 
1. Reflash the [normal firmware](https://github.com/MMMZZZZ/Syntherrupter/releases). No other settings have been changed. 
