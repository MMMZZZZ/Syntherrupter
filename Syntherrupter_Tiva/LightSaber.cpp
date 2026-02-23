/*
 * LightSaber.cpp
 *
 *  Created on: 06.09.2020
 *      Author: Max Zuidberg
 */


#include <LightSaber.h>


bool     LightSaber::modeRunning = false;
uint32_t LightSaber::lastPacket = 0;
uint32_t LightSaber::lastLSUpdateUS = 0;
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

    // Tweak the individual lightsabers such that they sound differently.
    // One of them (nbr 0) can remain unchanged.
    lightsabers[1].buzz.baseVolume *= 0.5f;
    lightsabers[1].hum2.baseVolume *= 0.8f;

    lightsabers[2].buzz.baseVolume     *= 0.2f;
    lightsabers[2].hum2.baseVolume      = 0.3f;
    lightsabers[2].hum2.volumeLFO.amp   = 0.2f;
    lightsabers[2].hum2.volumeLFO.freq  = 3.2f;

    lightsabers[3].buzz.baseVolume     *= 0.1f;
    lightsabers[2].buzz.volumeLFO.freq  = 1.7f;
    lightsabers[3].hum2.baseVolume     *= 0.8f;

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
        }

        if (timeUS - lastLSUpdateUS > 3000)
        {
            lastLSUpdateUS = timeUS;
            for (uint32_t lsNum = 0; lsNum < MAX_CLIENTS; lsNum++)
            {
                lightsabers[lsNum].process();
            }
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

        if (modeRunning && ls->assignedCoils & coilBit && timeUS - ls->lastReceive < 100000)
        {
            if (ls->updated)
            {
                float ontimeUS;
                float periodUS;

                ontimeUS = this->ontimeUS * ls->buzz.volume;
                periodUS = 1e6f / ls->buzz.frequency;
                tonelist->updateTone<ToneList::Owner::LIGHTSABER>(0 * MAX_CLIENTS + lsNum, Tone::Type::dflt, ontimeUS, periodUS, 0, 0);
                ontimeUS = this->ontimeUS * ls->hum1.volume;
                periodUS = 1e6f / ls->hum1.frequency;
                tonelist->updateTone<ToneList::Owner::LIGHTSABER>(1 * MAX_CLIENTS + lsNum, Tone::Type::dflt, ontimeUS, periodUS, 0, 0);
                ontimeUS = this->ontimeUS * ls->hum2.volume;
                periodUS = 1e6f / ls->hum2.frequency;
                tonelist->updateTone<ToneList::Owner::LIGHTSABER>(2 * MAX_CLIENTS + lsNum, Tone::Type::dflt, ontimeUS, periodUS, 0, 0);
            }
        }
        else
        {
            // This coil is no more listening to this lightsaber or the connection is dead.
            // Remove the assigned tone if there is one.
            tonelist->updateTone<ToneList::Owner::LIGHTSABER>(0 * MAX_CLIENTS + lsNum, Tone::Type::dflt, 0, 0, 0, 0);
            tonelist->updateTone<ToneList::Owner::LIGHTSABER>(1 * MAX_CLIENTS + lsNum, Tone::Type::dflt, 0, 0, 0, 0);
            tonelist->updateTone<ToneList::Owner::LIGHTSABER>(2 * MAX_CLIENTS + lsNum, Tone::Type::dflt, 0, 0, 0, 0);
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
