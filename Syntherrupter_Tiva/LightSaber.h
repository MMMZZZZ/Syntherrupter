/*
 * LightSaber.h
 *
 *  Created on: 06.09.2020
 *      Author: Max Zuidberg
 */


#ifndef LIGHTSABER_H_
#define LIGHTSABER_H_


#include <stdbool.h>
#include <stdint.h>
#include "InterrupterConfig.h"
#include "ToneList.h"
#include "UART.h"
#include "LSData.h"


class LightSaber
{
public:
    LightSaber();
    virtual ~LightSaber();
    static void init(uint32_t uartPort, uint32_t uartRxPin, uint32_t uartTxPin, uint32_t baudRate, void (*rxISR)(void));
    static void process();
    static void start();
    static void stop();
    static void ESPSetID(uint32_t id);
    void updateTonelist();
    void setTonelist(ToneList* tonelist)
    {
        this->tonelist = tonelist;
    }
    void setOntimeUS(float ontimeUS)
    {
        this->ontimeUS = ontimeUS;
        this->coilChange = true;
    }
    void setActiveLightsabers(uint32_t lsBits)
    {
        for (uint32_t lsNum = 0; lsNum < MAX_CLIENTS; lsNum++)
        {
            if (lsBits & (1 << lsNum))
            {
                lightsabers[lsNum].assignedCoils |=  coilBit;
            }
            else
            {
                lightsabers[lsNum].assignedCoils &= ~coilBit;
            }
        }
    };
    void setCoilNum(uint32_t num)
    {
        this->coilNum = num;
        this->coilBit = 1 << num;
    }
    float getOntimeUS()
    {
        return ontimeUS;
    }
    static UART uart;

private:
    static bool ESPCommand(uint8_t address, uint8_t data);
    static constexpr uint32_t MAX_CLIENTS = 4;
    static constexpr uint32_t DATA_SIZE = 24;
    static constexpr uint32_t PACKET_TIMEOUT_US = 30000 / MAX_CLIENTS / 2; // Each client sends a packet every 20ms. /2 to be sure delay is small enough.
    static constexpr uint32_t ESP_CMD_TIMEOUT_US = 9000;

    // Wild guess
    static constexpr uint32_t ESP_START_TIMEOUT_US = 20000;

    /*
     * 2:       Safety factor
     * 140:     Approx. character count of the bootloader message
     * 10:      Bits per Character (1 Start, 8 Data, 1 Stop Bit)
     * 1000000: us per s
     * 74880:   Baud rate
     */
    static constexpr uint32_t ESP_START_MSG_DURATION_US = 2ul * 140ul * 10ul * 1000000ul / 74880ul;

    static uint32_t lastPacket;
    static LSData lightsabers[MAX_CLIENTS];
    static bool started;

    ToneList* tonelist;
    uint32_t coilNum = 0;
    uint32_t coilBit = 0;
    bool coilChange = false;
    float ontimeUS = 0.0f;
};


#endif /* LIGHTSABER_H_ */
