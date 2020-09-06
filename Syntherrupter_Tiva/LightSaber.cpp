/*
 * LightSaber.cpp
 *
 *  Created on: 06.09.2020
 *      Author: Max Zuidberg
 */


#include <LightSaber.h>


bool   LightSaber::started = false;
UART   LightSaber::uart;
LSData LightSaber::lightsabers[MAX_CLIENTS];


LightSaber::LightSaber()
{
    // TODO Auto-generated constructor stub

}

LightSaber::~LightSaber()
{
    // TODO Auto-generated destructor stub
}

/*void LightSaber::init(uint32_t uartPort, uint32_t uartRxPin, uint32_t uartTxPin, void (*rxISR)(void))
{
    uart.init(uartPort, uartRxPin, uartTxPin, 115200, rxISR);
}*/

void LightSaber::start()
{
    started = true;
}

void LightSaber::stop()
{
    started = false;
}

void LightSaber::process()
{
    if (started)
    {
        float data[6];
        uint32_t target = 0;
        if (uart.buffer.level() >= DATA_SIZE + 1)
        {
            uint8_t dataByte = uart.buffer.read();
            if (dataByte && dataByte < MAX_CLIENTS)
            {
                target = dataByte;

                // Data is originally an array of 6 floats, transmitted byte by byte.
                for (uint32_t i = 0; i < 24; i++)
                {
                    ((uint8_t*) ((void*) data))[i] = uart.buffer.read();
                }
            }
        }

        if (target)
        {
            lightsabers[target].setData(data);
            lightsabers[target].process();
        }
    }
}

void LightSaber::updateTonelist()
{
    // Slightly inspired by the MIDI::updateTonelist method.

    Tone* lastTone = (Tone*) 1;
    uint32_t timeUS = System::getSystemTimeUS();
    for (uint32_t lsNum = 0; lsNum < MAX_CLIENTS; lsNum++)
    {
        LSData* ls = &(lightsabers[lsNum]);
        if (ls->changed & coilBit || coilChange)
        {
            ls->changed &= ~coilBit;

            Tone** assignedTone = &(ls->assignedTones[coilNum]);

            if (started && timeUS - ls->lastUpdate < 1000000 && ls->assignedCoils & coilBit)
            {
                if (lastTone)
                {
                    float ontimeUS = ls->volume * this->ontimeUS;
                    if (*assignedTone)
                    {
                        if ((*assignedTone)->owner != this)
                        {
                            *assignedTone = 0;
                        }
                    }
                    if (ontimeUS < 1.0f)
                    {
                        if (*assignedTone)
                        {
                            (*assignedTone)->remove(ls);
                            *assignedTone = 0;
                        }
                    }
                    else
                    {
                        lastTone = tonelist->updateTone(ontimeUS, ls->periodUS, this, ls, *assignedTone);
                        *assignedTone = lastTone;
                    }
                }
            }
            else
            {
                // This coil is no more listening to this channel. Remove the
                // assigned tone if there is one.
                if (*assignedTone)
                {
                    (*assignedTone)->remove(ls);
                    *assignedTone = 0;
                }
            }
        }
    }
    coilChange     = false;
}
