/*
 * main.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */


#include "InterrupterConfig.h"
#include "System.h"
#include "GUI.h"


System sys;
GUI gui;


/*
 * Awful. Still coudn't find a better way to use ISRs in C++.
 */
void sysTickISR()
{
    sys.systemTimeIncrement();

}

void midiUartISR()
{
    gui.midiUartISR();
}

void oneshotISR0()
{
    gui.coils[0].one.rearmISR();
}
void oneshotISR1()
{
    gui.coils[1].one.rearmISR();
}
void oneshotISR2()
{
    gui.coils[2].one.rearmISR();
}
void oneshotISR3()
{
    gui.coils[3].one.rearmISR();
}
void oneshotISR4()
{
    gui.coils[4].one.rearmISR();
}
void oneshotISR5()
{
    gui.coils[5].one.rearmISR();
}

void (*oneshotISRs[6])(void) = {oneshotISR0, oneshotISR1, oneshotISR2, oneshotISR3, oneshotISR4, oneshotISR5};

int main(void)
{
    sys.init(120000000, sysTickISR);
    sys.setSystemTimeResUS(1000);
    gui.init(&sys, midiUartISR, oneshotISRs);

    while (42)
    {
        gui.update();
    }
}

