# Optical Outputs

[Back to the hardware documentation overview](README.md#readme)

The connection between interrupter and tesla coil driver is usually made with fibre optics. This page explains how to add optic transmitters to Syntherrupter. 

Syntherrupter can generate up to 6 independant interrupter signals (for up to 6 independent tesla coils). You're completely free in the choice of how many outputs you want to have on your Syntherrupter; 1, 2, 3, 4, 5 or 6 are possible. In any case you'll need one transmitter per output. 

## Output Pins and Limits

The 6 output signals are available on the following Tiva pins:

* PD0: Output 1
* PD2: Output 2
* PM0: Output 3
* PM2: Output 4
* PM4: Output 5
* PM6: Output 6

All output pins are 3.3V active high and capable of 12mA max. Depending on your transmitter this is not enough to drive its LED. Please check the datasheet for the recommended current. In case your transmitter needs more current (the HFBR-14E4Z f.ex. needs 60mA) you'll need some additional circuitry. Nothing complicated, a standard NPN transistors and 2 resistors do the job. As always, details in the wiring and schematic folder linked at the top.

## Schematic

The following Schematic of a "complete" Syntherrupter shows how to connect the NPN transistors and what resistor values are needed. **Remember** to adjust the LED resistor for your current! The resistor value can easily be calculated using the following formula: `R_LED = (V_CC - V_CE - V_f) / I_f = (4.4 - V_f) / I_f`

![Complete Schematic](/Documentation/Wiring%20and%20Schematics/Syntherrupter%20Complete/Syntherrupter%20Complete%20Schematic.png)
