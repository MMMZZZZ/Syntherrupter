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


void sysTickISR()
{
    sys.systemTimeIncrement();

}

void midiUartISR()
{
    gui.midiUartISR();
}

int main(void)
{
    sys.init(120000000, sysTickISR);
    sys.setSystemTimeResUS(100);
    gui.init(&sys, midiUartISR);

    while (42)
    {
        gui.update();
    }
}

