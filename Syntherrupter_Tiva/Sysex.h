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
    static constexpr uint32_t WILDCARD        = 127;
    static constexpr uint32_t MODE_SIMPLE     = 1;
    static constexpr uint32_t MODE_MIDI_LIVE  = 2;
    static constexpr uint32_t MODE_LIGHTSABER = 3;

    static Nextion* nxt;
    static uint32_t uiUpdateMode;
};


#endif /* SYSEX_H_ */
