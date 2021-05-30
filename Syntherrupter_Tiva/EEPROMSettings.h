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
#include "string.h"
#include <algorithm>
#include "driverlib/sysctl.h"
#include "driverlib/eeprom.h"
#include "System.h"
#include "Coil.h"
#include "MIDI.h"
#include "MIDIProgram.h"


class EEPROMSettings
{
public:
    static constexpr uint32_t STR_CHAR_COUNT = 32; // must be multiple of 4
    struct UserData
    {
        char name[STR_CHAR_COUNT + 1];
        char password[STR_CHAR_COUNT + 1];
        uint16_t maxOntimeUS;
        uint16_t maxBPS;
        uint16_t maxDutyPerm;
    };
    struct CoilData
    {
        bool outputInvert;
        uint32_t minOntimeUS;
        uint32_t minOfftimeUS;
        uint32_t maxOntimeUS;
        uint32_t maxDutyPerm;
        float simpleOntimeFF;
        float simpleOntimeFC;
        float simpleBPSFF;
        float simpleBPSFC;
        uint32_t midiMaxVoices;
    };
    struct DeviceData
    {
        uint8_t  deviceID;
        uint8_t  eepromUpdateMode;
        uint8_t  uiBrightness;
        uint8_t  uiColorMode;
        uint16_t uiButtonHoldTime;
        uint16_t uiSleepDelay;
        bool     uiBackOff;
        float    midiLfoPeriodUS;
    };

    static constexpr uint32_t NO_CFG       = 0;
    static constexpr uint32_t CFG_OK       = 1;
    static constexpr uint32_t CFG_UPGRADED = 2;
    static constexpr uint32_t CFG_UNKNOWN  = 3;

    EEPROMSettings();
    virtual ~EEPROMSettings();
    static uint32_t init();
    static void readAll();
    static void writeAll();
    static void updateAll();
    static uint8_t& version;
    static UserData (&userData)[3];
    static CoilData (&coilData)[6];
    static DeviceData &deviceData;
private:
    static uint32_t legacyImport();
    static void initDefault();
    static void rwuAll(uint32_t mode);
    static bool rwuSingle(uint32_t mode, void *newData, uint32_t byteSize);
    static void readSequence(void *newData, uint32_t byteSize);
    static void writeSequence(void *newData, uint32_t byteSize);
    static bool writeChangedSequence(void *newData, uint32_t byteSize);

    static constexpr uint32_t PRESENT       = 0x42;
    static constexpr uint32_t VERSION       = 0x03;

    static constexpr uint32_t WRITE_EEPROM  = 0;
    static constexpr uint32_t READ_EEPROM   = 1;
    static constexpr uint32_t UPDATE_EEPROM = 2;

    static constexpr uint32_t PAGE_COUNT = 96;
    static constexpr uint32_t PAGE_SIZE  = 64;

    static constexpr uint32_t ENV_PROG_COUNT  = 20; // Only 20 programs are stored in EEPROM.
    static constexpr uint32_t ENV_PROG_OFFSET = 20; // First 20 programs are built-in defaults.

    static uint32_t byteAddress;

    // EEPROM Layouts (current and past ones
    struct CurrentLayout
    {
        // All parameters that shall be stored in EEPROM
        // List all parameters stored in EEPROM here:
        uint16_t reserved;
        uint8_t version;
        uint8_t present;
        MIDIProgram::DataPoint envelopes[ENV_PROG_COUNT][MIDIProgram::DATA_POINTS];
        UserData userData[3];
        CoilData coilData[6];
        DeviceData deviceData;
    };

    /*
     *  Legacy layouts.
     */

    // v2 stored envelope data as float and/or uint32 (no structs)
    union LegacyV2EnvelopeData
    {
        float     f32;
        uint32_t ui32;
    };
    // Constants used in old version
    static constexpr uint32_t V2_STR_CHAR_COUNT  = 32;
    static constexpr uint32_t V2_ENV_PROG_COUNT  = 20;
    static constexpr uint32_t V2_ENV_DATA_POINTS =  8;
    // v2 used 2 Banks
    union LegacyV2Bank
    {
        struct
        {
            uint16_t wear;
            uint8_t version;
            uint8_t present;
            char userNames[3][V2_STR_CHAR_COUNT];
            char userPwds[3][V2_STR_CHAR_COUNT];
            uint32_t userSettings[3];
            uint32_t coilSettings[6];
            /*
             * Note: otherSettings was originally intended to be 10 fields in
             * size but due to a bug only 6 fields were actually stored in
             * EEPROM. Since only the first 2 were ever used, this is no real
             * problem. It's just an explanation for the 6 instead of a 10.
             */
            uint32_t otherSettings[6];
            LegacyV2EnvelopeData envelopes[V2_ENV_PROG_COUNT][V2_ENV_DATA_POINTS][4];
        } data;

        uint8_t raw[3072];
    };
    struct LegacyV2Layout
    {
        LegacyV2Bank bank0;
        LegacyV2Bank bank1;
    };


    union EEPROMData
    {
        // All Layouts (each one being exactly 6144 bytes in size)
        CurrentLayout data;
        LegacyV2Layout legacyV2;

        // The Array that represents the entire EEPROM (6144 bytes)
        uint8_t raw[PAGE_COUNT][PAGE_SIZE];


    };

    static EEPROMData eeprom;
    static_assert(sizeof(eeprom) <= 6144, "EEPROM struct exceeds actual EEPROM size.");

    // Some Data is not stored in EEPROM but still requires EEPROMSettings
    // to provide the memory locations.
    struct VolatileData
    {
        // Remaining envelopes that are not stored in EEPROM.
        MIDIProgram::DataPoint envelopes[MIDI::MAX_PROGRAMS - ENV_PROG_COUNT][MIDIProgram::DATA_POINTS];
    };

    static VolatileData volatileData;

    // Default values for current version
    static CurrentLayout defaultSettings;
};

#endif /* EEPROMSETTINGS_H_ */
