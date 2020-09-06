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
    static void init(uint32_t uartPort, uint32_t uartRxPin, uint32_t uartTxPin, void (*rxISR)(void));
    static void process();
    static void start();
    static void stop();
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
                lightsabers[lsNum].assignedCoils |=  (1 << lsNum);
            }
            else
            {
                lightsabers[lsNum].assignedCoils &= ~(1 << lsNum);
            }
        }
    };
    void setCoilNum(uint32_t num)
    {
        this->coilNum = num;
        this->coilBit = 1 << num;
    }
    static UART uart;

private:
    static constexpr uint32_t MAX_CLIENTS = 8;
    static constexpr uint32_t DATA_SIZE = 24;

    static LSData lightsabers[MAX_CLIENTS];
    static bool started;

    ToneList* tonelist;
    uint32_t coilNum = 0;
    uint32_t coilBit = 0;
    bool coilChange = false;
    float ontimeUS = 0.0f;
};


#endif /* LIGHTSABER_H_ */
