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
#include "Settings.h"
#include "Coil.h"
#include "GUI.h"


void sysTickISR()
{
    System::systemTimeIncrement();
}

void uartUsbISR()
{
    MIDI::usbUart.ISR();
}

void uartMidiISR()
{
    MIDI::midiUart.ISR();
}

void uartLightSaberISR()
{
    LightSaber::uart.ISR();
}


int main(void)
{
    System::init(sysTickISR);
    System::setSystemTimeResUS(16);

    MIDI::init(115200, uartUsbISR, GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_5, uartMidiISR);
    LightSaber::init(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_7, 115200, uartLightSaberISR);

    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        Coil::allCoils[coil].init(coil);
    }

    GUI::init();

    while (42)
    {
        uint32_t state = GUI::update();

        if (state)
        {
            MIDI::process();
            Settings::processSysex();
            LightSaber::process();

            switch (COIL_COUNT)
            {
                case 6:
                    Coil::allCoils[5].updateData();
                    Coil::allCoils[5].updateOutput();
                case 5:
                    Coil::allCoils[4].updateData();
                    Coil::allCoils[4].updateOutput();
                case 4:
                    Coil::allCoils[3].updateData();
                    Coil::allCoils[3].updateOutput();
                case 3:
                    Coil::allCoils[2].updateData();
                    Coil::allCoils[2].updateOutput();
                case 2:
                    Coil::allCoils[1].updateData();
                    Coil::allCoils[1].updateOutput();
                case 1:
                    Coil::allCoils[0].updateData();
                    Coil::allCoils[0].updateOutput();
                    break;
            }
            /*for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                // Run non-static coil object methods
                Coil::allCoils[coil].update();
            }*/
        }
        else
        {
            // Emergency stop or something similar. Don't generate outputs
            // and delete all tones
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                ToneList* tl = &(Coil::allCoils[coil].toneList);
                for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
                {
                    tl->deleteTone(tl->firstTone);
                }
            }
        }
    }
}

