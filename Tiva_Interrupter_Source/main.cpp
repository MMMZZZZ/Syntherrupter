/*
 * main.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */


#include <Output.h>
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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlDelay(3);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
    SysCtlDelay(sys.getClockFreq() / 9);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

    gui.init(&sys, midiUartISR);

    while (42)
    {
        if (!gui.update())
        {
            gui.showError();
            sys.error();
        }
        gui.applyOutput();

    }
}

