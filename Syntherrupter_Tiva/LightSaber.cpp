/*
 * LightSaber.cpp
 *
 *  Created on: 06.09.2020
 *      Author: Max Zuidberg
 */


#include <LightSaber.h>


bool     LightSaber::modeRunning = false;
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
        if (uart.rxBuffer.level() > 10)
        {
            // Message has started
            System::delayUS(ESP_START_MSG_DURATION_US);
            break;
        }
    }
    uart.rxBuffer.flush();
    uart.disable();
}

void LightSaber::setRunning(bool run)
{
    modeRunning = run;
    if (run)
    {
        uart.enable();
    }
    else
    {
        uart.disable();
        lastPacket = 0;
    }
}

void LightSaber::process()
{
    static_assert(MAX_CLIENTS <= TONE_COUNT_LS, "LS tone count must be higher than ls client count (min 1 tone per client).");
    if (modeRunning)
    {
        uint32_t timeUS = System::getSystemTimeUS();
        static uint32_t lastBufferLevel{0};
        static uint32_t lastTimeUS{0};
        uint32_t newBufferLevel = uart.rxBuffer.level();
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
            uart.rxBuffer.flush();
            lastTimeUS = timeUS;
        }

        if (uart.rxBuffer.level() >= DATA_SIZE + 1)
        {
            uint8_t dataByte = uart.rxBuffer.read();
            if (dataByte && dataByte < MAX_CLIENTS)
            {
                lastPacket = timeUS;
                target = dataByte;

                // Data is originally an array of 6 floats, transmitted byte by byte.
                for (uint32_t i = 0; i < 24; i++)
                {
                    ((uint8_t*) ((void*) data))[i] = uart.rxBuffer.read();
                }
            }
            else
            {
                uart.rxBuffer.flush();
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

    uint32_t timeUS = System::getSystemTimeUS();
    for (uint32_t lsNum = 0; lsNum < MAX_CLIENTS; lsNum++)
    {
        LSData* ls = &(lightsabers[lsNum]);

        if (ls->changed & coilBit || coilChange)
        {
            ls->changed &= ~coilBit;

            if (modeRunning && ls->assignedCoils & coilBit)
            {
                float ontimeUS = ls->volume * this->ontimeUS;
                tonelist->updateTone<ToneList::Owner::LIGHTSABER>(lsNum, Tone::Type::dflt, ontimeUS, ls->periodUS, 0, 0);
            }
            else
            {
                // This coil is no more listening to this lightsaber. Remove the
                // assigned tone if there is one.
                tonelist->updateTone<ToneList::Owner::LIGHTSABER>(lsNum, Tone::Type::dflt, 0, 0, 0, 0);
            }
        }
        else if (timeUS - ls->lastUpdate >= 100000)
        {
            // Dead. remove.
            tonelist->updateTone<ToneList::Owner::LIGHTSABER>(lsNum, Tone::Type::dflt, 0, 0, 0, 0);
        }
    }
    coilChange = false;
}

bool LightSaber::ESPCommand(uint8_t address, uint8_t data)
{
    uart.rxBuffer.flush();
    uart.sendChar(address);
    uart.sendChar(data);

    // For now we don't care about the return data.
    uint32_t startTimeUS = System::getSystemTimeUS();
    while (System::getSystemTimeUS() - startTimeUS < ESP_CMD_TIMEOUT_US)
    {
        if (uart.rxBuffer.level() == 1)
        {
            if (uart.rxBuffer.read() == data)
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
