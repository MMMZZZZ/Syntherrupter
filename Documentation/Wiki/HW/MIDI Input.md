# MIDI Input Jack

[Back to the hardware documentation overview](README.md#readme)

If you want to receive MIDI data using the standard 5-pin DIN connector, you'll need to add the according jack for it. 

## Communication Pin

* Tiva PC4: MIDI In

## Schematic

The following schematic of a "complete" Syntherrupter shows the circuitry needed to receive MIDI data. The way how the transmission works makes it mandatory to use an optocoupler on the receiver side. R13 is not critical and can be pretty much anything from 1k to 10k.

![Complete Schematic](/Documentation/Wiring%20and%20Schematics/Syntherrupter%20Complete/Syntherrupter%20Complete%20Schematic.png)
