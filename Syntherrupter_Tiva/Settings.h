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
#include "InterrupterConfig.h"
#include "Coil.h"
#include "Simple.h"
#include "MIDI.h"
#include "LightSaber.h"
#include "EEPROMSettings.h"


class Settings
{
public:
    Settings();
    static void init();
    static void processSysex();
private:
    static bool checkSysex(uint32_t number, uint32_t targetLSB, uint32_t targetMSB, int32_t sysexVal);
    static constexpr uint32_t MODE_ALL        = 127;
    static constexpr uint32_t MODE_SIMPLE     = 1;
    static constexpr uint32_t MODE_MIDI_LIVE  = 2;
    static constexpr uint32_t MODE_LIGHTSABER = 3;

    static uint32_t eepromUpdateMode;
};


#endif /* SETTINGS_H_ */
