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
#include "Coil.h"
#include "MIDI.h"
#include "MIDIProgram.h"

#define ANY_TO_UINT32(ANY) *((uint32_t*) ((void *) (&ANY)))
#define ANY_TO_FLOAT(ANY)  *((float*)    ((void *) (&ANY)))


class EEPROMSettings
{
public:
    static constexpr uint32_t STR_CHAR_COUNT = 32; // must be multiple of 4
    struct UserData
    {
        char name[STR_CHAR_COUNT + 1];
        char password[STR_CHAR_COUNT + 1];
        float maxOntimeUS;
        float maxBPS;
        float maxDutyPerm;
    };
    struct CoilData
    {
        uint32_t minOntimeUS;
        uint32_t minOfftimeUS;
        uint32_t maxOntimeUS;
        uint32_t maxDutyPerm;
        uint32_t maxMidiVoices;
        float simpleOntimeFF;
        float simpleOntimeFC;
        float simpleBPSFF;
        float simpleBPSFC;
    };
    struct UIData
    {
        uint16_t buttonHoldTime;
        uint16_t sleepDelay;
        uint8_t brightness;
        uint8_t colorMode;
    };
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
    static UserData (&userData)[3];
    static CoilData (&coilData)[6];
    static UIData &uiData;
private:
    static void rwuAll(uint32_t mode);
    static void rwuAllLegacyV2(uint32_t mode);
    static bool rwuSingle(uint32_t mode, void *newData, uint32_t byteSize);
    static void readSequence(void *newData, uint32_t byteSize);
    static void writeSequence(void *newData, uint32_t byteSize);
    static bool writeChangedSequence(void *newData, uint32_t byteSize);
    static bool updateBank();

    static constexpr uint32_t PRESENT       = 0x42020000; // MSB: Random value != 0 and != 0xff to check if data has been written to the EEPROM. Next byte: Config Version. Increments if there has been incompatible changes. Lowest two bytes: wear leveling. Switch to next bank on overflow.
    static constexpr uint32_t WRITE_EEPROM  = 0;
    static constexpr uint32_t READ_EEPROM   = 1;
    static constexpr uint32_t UPDATE_EEPROM = 2;

    static constexpr uint32_t BANK_COUNT    = 2;
    static constexpr uint32_t BANK_STARTS[BANK_COUNT]  = {0, 3072};

    static constexpr uint32_t PAGE_COUNT = 96;
    static constexpr uint32_t PAGE_SIZE  = 16;

    static constexpr uint32_t ENV_AMP  = 0;
    static constexpr uint32_t ENV_DUR  = 1;
    static constexpr uint32_t ENV_NTAU = 2;
    static constexpr uint32_t ENV_NEXT = 3;
    static constexpr uint32_t ENV_PROG_COUNT  = 20; // Only 20 programs are stored in EEPROM.
    static constexpr uint32_t ENV_PROG_OFFSET = 20; // First 20 programs are build-in defaults.

    // LEGACY
    static uint32_t EnvelopeSettings[ENV_PROG_COUNT][MIDIProgram::DATA_POINTS][4];
    static char userNames[3][STR_CHAR_COUNT];
    static char userPwds[3][STR_CHAR_COUNT];
    static uint32_t userSettings[3];
    static uint32_t coilSettings[6];
    static uint32_t otherSettings[10];
    // END LEGACY

    static uint32_t byteAddress;
    static uint32_t bank; // initialized to value higher than normally possible
    static bool     EEPROMUpToDate;

    // EEPROM Layouts (current and past ones
    struct CurrentLayout
    {
        // All parameters that shall be stored in EEPROM
        // List all parameters stored in EEPROM here:
        uint32_t present;
        MIDIProgram::DataPoint envelopes[ENV_PROG_COUNT][MIDIProgram::DATA_POINTS];
        UserData userData[3];
        CoilData coilData[6];
        UIData uiData;
    };

    // Legacy layouts.

    // V2 used 2 Banks
    union LegacyV2Bank
    {
        struct
        {
            uint32_t present;
            char userNames[3][STR_CHAR_COUNT];
            char userPwds[3][STR_CHAR_COUNT];
            uint32_t userSettings[3];
            uint32_t coilSettings[6];
            uint32_t otherSettings[10];
            uint32_t EnvelopeSettings[ENV_PROG_COUNT][MIDIProgram::DATA_POINTS][4];
        } parameters;

        uint8_t bankSize[3072];
    };
    struct LegacyV2Layout
    {
        LegacyV2Bank bank0;
        LegacyV2Bank bank1;
    };


    union EEPROMData
    {
        // Layout of the current version
        CurrentLayout parameters;
        LegacyV2Layout legacyV2;

        // The Array that represents the entire EEPROM
        uint8_t raw[PAGE_COUNT][PAGE_SIZE];


    };

    static EEPROMData eeprom;
};

#endif /* EEPROMSETTINGS_H_ */
