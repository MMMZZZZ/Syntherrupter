/*
 * main.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */


#include <stdint.h>
#include <stdbool.h>
#include "InterrupterConfig.h"
#include "System.h"
#include "GUI.h"


System sys;
Coil coils[COIL_COUNT];

extern "C"
{
void sysTickISR()
{
    sys.systemTimeIncrement();
}

void uartUsbISR()
{
    coils->midi.usbUart.ISR();
}

void uartMidiISR()
{
    coils->midi.midiUart.ISR();
}
}

int main(void)
{
    GUI gui;
    sys.init(sysTickISR);
    sys.setSystemTimeResUS(10);
    gui.init();

    // Initialize Coil objects
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        coils[coil].init(coil);
    }
    coils->midi.init(115200, uartUsbISR, GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_5, uartMidiISR);

    while (42)
    {
        uint32_t state = gui.update();
        if (state)
        {
            // Use coils[0] objects for calling their static methods.
            coils->midi.process();

            // Run non-static coil object methods
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                coils[coil].midi.updateToneList();
            }

            // Generate output
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                coils[coil].output();
            }
        }
        else
        {
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
                {
                    ToneList* tl = &(coils[coil].toneList);
                    tl->deleteTone(tl->tones[tone]);
                }
            }
        }
    }
}

