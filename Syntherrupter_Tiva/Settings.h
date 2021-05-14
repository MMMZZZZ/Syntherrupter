/*
 * Settings.h
 *
 *  Created on: 10.04.2021
 *      Author: Max Zuidberg
 */


#ifndef SETTINGS_H_
#define SETTINGS_H_


#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "InterrupterConfig.h"
#include "Nextion.h"
#include "Coil.h"
#include "Sysex.h"
#include "GUI.h"
#include "Simple.h"
#include "MIDI.h"
#include "LightSaber.h"
#include "EEPROMSettings.h"


class Settings
{
public:
    Settings();
    static void init(Nextion* nextion);
    static void processSysex();
private:
    static bool checkSysex(SysexMsg& msg);
    static constexpr uint32_t WILDCARD        = 127;
    static constexpr uint32_t MODE_SIMPLE     = 1;
    static constexpr uint32_t MODE_MIDI_LIVE  = 2;
    static constexpr uint32_t MODE_LIGHTSABER = 3;

    static uint32_t eepromUpdateMode;

    static Nextion* nxt;
};


#endif /* SETTINGS_H_ */
