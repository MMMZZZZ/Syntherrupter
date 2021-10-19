/*
 * Sysex.h
 *
 *  Created on: 10.04.2021
 *      Author: Max Zuidberg
 */


#ifndef SYSEX_H_
#define SYSEX_H_


#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "InterrupterConfig.h"
#include "Nextion.h"
#include "Coil.h"
#include "GUI.h"
#include "Simple.h"
#include "MIDI.h"
#include "LightSaber.h"
#include "EEPROMSettings.h"
#include "SysexMsg.h"

class Sysex
{
public:
    Sysex();
    static void init(Nextion* nextion);
    static void processSysex();
private:
    static bool checkSysex(SysexMsg& msg);
    static void sendSysex();
    static constexpr uint32_t WILDCARD        = 127;
    static constexpr uint32_t MODE_SIMPLE     = 1;
    static constexpr uint32_t MODE_MIDI_LIVE  = 2;
    static constexpr uint32_t MODE_LIGHTSABER = 3;

    static Nextion* nxt;
    static uint32_t uiUpdateMode;
    static SysexMsg msg;
    static bool reading;
    static bool readFloat;
    static bool readSupportOnly;

    union TxMsg {
        struct
        {
            uint8_t START;
            uint8_t DMID_0;
            uint8_t DMID_1;
            uint8_t DMID_2;
            uint8_t version;
            uint8_t deviceID;
            uint16_t number;
            uint8_t targetLSB;
            uint8_t targetMSB;
            uint8_t splitValue[5];
            uint8_t END;
        } data;
        uint8_t serialized[16];
    };

    static TxMsg txMsg;
};


#endif /* SYSEX_H_ */
