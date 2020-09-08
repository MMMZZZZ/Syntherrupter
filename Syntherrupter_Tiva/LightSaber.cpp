/*
 * LightSaber.cpp
 *
 *  Created on: 06.09.2020
 *      Author: Max Zuidberg
 */


#include <LightSaber.h>


bool     LightSaber::started = false;
uint32_t LightSaber::lastPacket = 0;
UART     LightSaber::uart;
LSData   LightSaber::lightsabers[MAX_CLIENTS];


LightSaber::LightSaber()
{
    // TODO Auto-generated constructor stub

}

LightSaber::~LightSaber()
{
    // TODO Auto-generated destructor stub
}

void LightSaber::init(uint32_t uartPort, uint32_t uartRxPin, uint32_t uartTxPin, uint32_t baudRate, void (*rxISR)(void))
{
    uart.init(uartPort, uartRxPin, uartTxPin, 115200, rxISR);
    uart.enable();

    // Wait for ESP8266 bootloader output and discard it.
    uint32_t startTimeUS = System::getSystemTimeUS();
    while (System::getSystemTimeUS() - startTimeUS < ESP_START_TIMEOUT_US)
    {
        if (uart.buffer.level() > 10)
        {
            // Message has started
            System::delayUS(ESP_START_MSG_DURATION_US);
            break;
        }
    }
    uart.buffer.flush();
    uart.disable();
}

void LightSaber::start()
{
    started = true;
    uart.enable();
}

void LightSaber::stop()
{
    started = false;
    uart.disable();
    lastPacket = 0;
}

void LightSaber::process()
{
    if (started)
    {
        uint32_t timeUS = System::getSystemTimeUS();
        static uint32_t lastBufferLevel{0};
        static uint32_t lastTimeUS{0};
        uint32_t newBufferLevel = uart.buffer.level();
        float data[6];
        uint32_t target = 0;

        // Packet alignment
        if (newBufferLevel != lastBufferLevel)
        {
            lastBufferLevel = newBufferLevel;
            lastTimeUS      = timeUS;
        }
        else if (timeUS - lastTimeUS > PACKET_TIMEOUT_US)
        {
            uart.buffer.flush();
            lastTimeUS = timeUS;
        }

        if (uart.buffer.level() >= DATA_SIZE + 1)
        {
            uint8_t dataByte = uart.buffer.read();
            if (dataByte && dataByte < MAX_CLIENTS)
            {
                lastPacket = timeUS;
                target = dataByte;

                // Data is originally an array of 6 floats, transmitted byte by byte.
                for (uint32_t i = 0; i < 24; i++)
                {
                    ((uint8_t*) ((void*) data))[i] = uart.buffer.read();
                }
            }
            else
            {
                uart.buffer.flush();
            }
        }

        if (target)
        {
            // Target goes from 1 - MAX_CLIENTS
            // Array Index goes from 0 - MAX_CLIENTS-1
            target--;
            lightsabers[target].setData(data);
            lightsabers[target].process();
        }
    }
}

void LightSaber::ESPSetID(uint32_t id)
{
    ESPCommand(1, id);
}

void LightSaber::updateTonelist()
{
    // Slightly inspired by the MIDI::updateTonelist method.

    Tone* lastTone = (Tone*) 1;
    uint32_t timeUS = System::getSystemTimeUS();
    for (uint32_t lsNum = 0; lsNum < MAX_CLIENTS; lsNum++)
    {
        LSData* ls = &(lightsabers[lsNum]);
        Tone** assignedTone = &(ls->assignedTones[coilNum]);

        if (ls->changed & coilBit || coilChange)
        {
            ls->changed &= ~coilBit;

            if (started && ls->assignedCoils & coilBit)
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
                // This coil is no more listening to this lightsaber. Remove the
                // assigned tone if there is one.
                if (*assignedTone)
                {
                    (*assignedTone)->remove(ls);
                    *assignedTone = 0;
                }
            }
        }
        else if (timeUS - ls->lastUpdate >= 100000)
        {
            // Dead. remove.
            if (*assignedTone)
            {
                (*assignedTone)->remove(ls);
                *assignedTone = 0;
            }
        }
    }
    coilChange = false;
}

bool LightSaber::ESPCommand(uint8_t address, uint8_t data)
{
    uart.buffer.flush();
    uart.sendChar(address);
    uart.sendChar(data);

    // For now we don't care about the return data.
    uint32_t startTimeUS = System::getSystemTimeUS();
    while (System::getSystemTimeUS() - startTimeUS < ESP_CMD_TIMEOUT_US)
    {
        if (uart.buffer.level() == 1)
        {
            if (uart.buffer.read() == data)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}
