/*
 * main.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */


#include <stdint.h>
#include <stdbool.h>
#include <Sysex.h>
#include <System.h>
#include "InterrupterConfig.h"
#include "EEPROMSettings.h"
#include "Nextion.h"
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

    //uint32_t cfgStatus = EEPROMSettings::init();

    //Nextion nextion;
    //bool nxtOk = nextion.init(3, 115200);

    //MIDI::init(115200, uartUsbISR, GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_5, uartMidiISR);
    //LightSaber::init(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_7, 115200, uartLightSaberISR);

    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        Coil::allCoils[coil].init(coil);
    }

    //GUI::init(&nextion, nxtOk, cfgStatus);
    //Sysex::init(&nextion);

    constexpr BUFFER_TIME_US = 10000;

    constexpr uint32_t P_C = 3;
    float periods[P_C] = {0.0f};
    periods[0] = 1 / 323.0f;
    periods[1] = periods[0] / 1.5f;
    periods[2] = periods[0] / 2.0f;
    float ontimes[P_C] = {0.0f};
    ontimes[0] = 100;
    ontimes[1] = 100;
    ontimes[2] = 100;

    uint32_t next[P_C] = {0};

    bool bufferSwap = false;
    uint32_t bufferNum = 0;

    while (42)
    {
        if (Coil::allCoils[0].out.currentBuffer == bufferNum)
        {
            // Buffer swapped, fill new empty buffer.
            bufferNum = !bufferNum;
            auto& buffer = bufferNum ? Coil::allCoils[0].out.buffer1 : Coil::allCoils[0].out.buffer0;
            uint32_t index = 0;
            for (uint32_t i = 0; i < P_C; i++)
            {
                if (System::getSystemTimeUS() >= next[i])
                {
                    next[i]
                }
                buffer[index]
            }
        }

        /*uint32_t state = GUI::update();

        if (state)
        {
            MIDI::process();
            Sysex::processSysex();
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
        }*/
    }
}

