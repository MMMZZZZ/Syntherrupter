/*
 * EEPROMSettings.h
 *
 *  Created on: 28.04.2020
 *      Author: Max Zuidberg
 */

#ifndef EEPROMSETTINGS_H_
#define EEPROMSETTINGS_H_


#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/eeprom.h"
#include "System.h"
#include "MIDI.h"

#define ANY_TO_UINT32(ANY) *((uint32_t*) ((void *) (&ANY)))
#define ANY_TO_FLOAT(ANY)  *((float*)    ((void *) (&ANY)))


class EEPROMSettings
{
public:
    EEPROMSettings();
    virtual ~EEPROMSettings();
    static bool init();
    static void read();
    static void write();
    static void update();
    static void setMIDIPrograms();
    static void getMIDIPrograms();
    static uint32_t getUsersMaxOntimeUS(uint32_t user);
    static uint32_t getUsersMaxBPS(uint32_t user);
    static uint32_t getUsersMaxDutyPerm(uint32_t user);
    static uint32_t getCoilsMaxOntimeUS(uint32_t coil);
    static uint32_t getCoilsMinOffUS(uint32_t coil);
    static uint32_t getCoilsMaxVoices(uint32_t coil);
    static uint32_t getCoilsMaxDutyPerm(uint32_t coil);
    static uint32_t getStngsHoldTime();
    static uint32_t getStngsBrightness();
    static constexpr uint32_t STR_CHAR_COUNT = 32; // must be multiple of 4
    static char userNames[3][STR_CHAR_COUNT];
    static char userPwds[3][STR_CHAR_COUNT];
    static uint32_t userSettings[3];
    static uint32_t coilSettings[6];
    static uint32_t otherSettings[10];
private:
    static void rwuAll(uint32_t mode);
    static bool rwuSingle(uint32_t mode, void *newData, uint32_t byteSize);
    static void readSequence(void *newData, uint32_t byteSize);
    static void writeSequence(void *newData, uint32_t byteSize);
    static bool writeChangedSequence(void *newData, uint32_t byteSize);
    static bool updateBank();

    static constexpr uint32_t PRESENT       = 0x42ff0000; // MSB: Random value != 0 and != 0xff to check if data has been written to the EEPROM. Next byte: Config Version. Increments if there has been incompatible changes. Lowest two bytes: wear leveling. Switch to next bank on overflow.
    static constexpr uint32_t WRITE_EEPROM  = 0;
    static constexpr uint32_t READ_EEPROM   = 1;
    static constexpr uint32_t UPDATE_EEPROM = 2;

    static constexpr uint32_t BANK_COUNT    = 2;
    static constexpr uint32_t BANK_STARTS[BANK_COUNT]  = {0, 3072};

    static constexpr uint32_t ADSR_AMP  = 0;
    static constexpr uint32_t ADSR_DUR  = 1;
    static constexpr uint32_t ADSR_NTAU = 2;
    static constexpr uint32_t ADSR_NEXT = 3;
    static constexpr uint32_t ADSR_PROG_COUNT  = 20; // Only 10 programs stored in EEPROM.
    static constexpr uint32_t ADSR_PROG_OFFSET = 20;
    static uint32_t ADSRSettings[ADSR_PROG_COUNT][MIDIProgram::DATA_POINTS][4];

    static uint32_t byteAddress;
    static uint32_t bank; // initialized to value higher than normally possible
    static uint32_t tempArray[(sizeof(ADSRSettings) > 30) ? MIDI::MAX_PROGRAMS : 30]; // must be at least as large as the largest array in use.
    static bool     EEPROMUpToDate;
};

#endif /* EEPROMSETTINGS_H_ */
