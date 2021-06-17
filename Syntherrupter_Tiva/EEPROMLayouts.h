/*
 * EEPROMLayouts.h
 *
 *  Created on: 17.06.2021
 *      Author: Max Zuidberg
 */

#ifndef EEPROMLAYOUTS_H_
#define EEPROMLAYOUTS_H_


#include "stdint.h"
#include "stdbool.h"


namespace LegacyV2
{
    static constexpr uint32_t STR_CHAR_COUNT  = 32;
    static constexpr uint32_t ENV_PROG_COUNT  = 20;
    static constexpr uint32_t ENV_DATA_POINTS =  8;

    // v2 stored envelope data as float and/or uint32 (no structs)
    union EnvelopeData
    {
        float     f32;
        uint32_t ui32;
    };
    // v2 used 2 Banks
    union Bank
    {
        struct
        {
            uint16_t wear;
            uint8_t version;
            uint8_t present;
            char userNames[3][STR_CHAR_COUNT];
            char userPwds[3][STR_CHAR_COUNT];
            uint32_t userSettings[3];
            uint32_t coilSettings[6];
            /*
             * Note: otherSettings was originally intended to be 10 fields in
             * size but due to a bug only 6 fields were actually stored in
             * EEPROM. Since only the first 2 were ever used, this is no real
             * problem. It's just an explanation for the 6 instead of a 10.
             */
            uint32_t otherSettings[6];
            EnvelopeData envelopes[ENV_PROG_COUNT][ENV_DATA_POINTS][4];
        } data;

        uint8_t raw[3072];
    };
    struct Layout
    {
        Bank bank0;
        Bank bank1;
    };
}

namespace LegacyV3
{
    static constexpr uint32_t STR_CHAR_COUNT  = 32;
    static constexpr uint32_t ENV_PROG_COUNT  = 20;
    static constexpr uint32_t ENV_DATA_POINTS =  8;

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
    struct MIDIProgramDataPoint
    {
        uint8_t nextStep;
        float durationUS;
        float amplitude;
        float ntau;
    };

    struct Layout
    {
        // All parameters that shall be stored in EEPROM
        // List all parameters stored in EEPROM here:
        uint16_t reserved;
        uint8_t version;
        uint8_t present;
        MIDIProgramDataPoint envelopes[ENV_PROG_COUNT][ENV_DATA_POINTS];
        UserData userData[3];
        CoilData coilData[6];
        DeviceData deviceData;
    };

    void setDefaultLayout(Layout& layout)
    {
        /*
         * Default "Header"
         */
        layout.present  = PRESENT;
        layout.version  = VERSION;
        layout.reserved = 1;

        /*
         * Default Coil Settings
         */
        for (uint32_t coil = 0; coil < 6; coil++)
        {
            layout.coilData[coil].maxDutyPerm    = 50;
            layout.coilData[coil].midiMaxVoices  =  8;
            layout.coilData[coil].maxOntimeUS    = 10;
            layout.coilData[coil].minOntimeUS    =  0;
            layout.coilData[coil].minOfftimeUS   = 10;
            layout.coilData[coil].outputInvert   = false;
            layout.coilData[coil].simpleBPSFC    = 5.0f;
            layout.coilData[coil].simpleBPSFF    = 1.8f;
            layout.coilData[coil].simpleOntimeFC = 30.0f;
            layout.coilData[coil].simpleOntimeFF = 2.0f;
        }

        /*
         * Default User Settings
         */
        layout.userData[0].maxBPS      = 100;
        layout.userData[0].maxDutyPerm = 10;
        layout.userData[0].maxOntimeUS = 10;
        strcpy(layout.userData[0].name, "Padawan");
        strcpy(layout.userData[0].password, "1234");
        layout.userData[1].maxBPS      = 200;
        layout.userData[1].maxDutyPerm = 20;
        layout.userData[1].maxOntimeUS = 20;
        strcpy(layout.userData[1].name, "Jedi Knight");
        strcpy(layout.userData[1].password, "8079");
        layout.userData[2].maxBPS      = 500;
        layout.userData[2].maxDutyPerm = 50;
        layout.userData[2].maxOntimeUS = 50;
        strcpy(layout.userData[2].name, "Master Yoda");
        strcpy(layout.userData[2].password, "0");

        /*
         * Default Device Settings
         */
        layout.deviceData.deviceID         = 0;
        layout.deviceData.eepromUpdateMode = 0; // Manual
        layout.deviceData.uiBackOff        = true;
        layout.deviceData.uiBrightness     = 100;
        layout.deviceData.uiButtonHoldTime = 250;
        layout.deviceData.uiColorMode      = 1; // Dark mode
        layout.deviceData.uiSleepDelay     = 0; // No sleep
        layout.deviceData.midiLfoPeriodUS  = 1.0f / 5.0f; // 5Hz

        /*
         * Default Envelopes
         */
        constexpr uint8_t LAST = MIDIProgram::DATA_POINTS - 1;

        // Program 0 and all unspecified programs: No envelope (constant 100% volume while on, no rise/fall times)
        for (uint8_t step = 0; step <= LAST; step++)
        {
            layout.envelopes[0][step] = {.amplitude = 1.0f, .durationUS = 1.0f, .ntau = 0.1f, .nextStep = (uint8_t) (step + 1)};
        }
        layout.envelopes[0][LAST - 1].nextStep = LAST - 1;
        layout.envelopes[0][LAST].amplitude    = 0.0f;
        layout.envelopes[0][LAST].nextStep     = LAST;

        // Initialize all EEPROM programs to the "empty" envelope
        for (uint32_t env = 1; env < ENV_PROG_COUNT; env++)
        {
            memcpy(layout.envelopes[env], layout.envelopes[0], sizeof(layout.envelopes[0]));
        }
    }
}


#endif /* EEPROMLAYOUTS_H_ */
