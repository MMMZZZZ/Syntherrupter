/*
 * EEPROMSettings.cpp
 *
 *  Created on: 28.04.2020
 *      Author: Max Zuidberg
 */

#include <EEPROMSettings.h>


//char     EEPROMSettings::userNames[3][STR_CHAR_COUNT] = {"Padawan", "Jedi Knight", "Master Yoda"};
//char     EEPROMSettings::userPwds[3][STR_CHAR_COUNT]  = {"1234",    "8079",        "0"};
uint32_t EEPROMSettings::byteAddress = 0;
EEPROMSettings::EEPROMData EEPROMSettings::eeprom;
uint8_t &EEPROMSettings::version = EEPROMSettings::eeprom.data.version;
EEPROMSettings::UserData (&EEPROMSettings::userData)[3] = EEPROMSettings::eeprom.data.userData;
EEPROMSettings::CoilData (&EEPROMSettings::coilData)[6] = EEPROMSettings::eeprom.data.coilData;
EEPROMSettings::DeviceData &(EEPROMSettings::deviceData) = EEPROMSettings::eeprom.data.deviceData;
EEPROMSettings::CurrentLayout EEPROMSettings::defaultSettings;
EEPROMSettings::VolatileData EEPROMSettings::volatileData;


uint32_t EEPROMSettings::init()
{
    // Make EEPROM available and initialize
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    SysCtlDelay(2);
    if (EEPROMInit() != EEPROM_INIT_OK)
    {
        // Power supply issues or EEPROM damaged / end of life
        System::error();
    }

    uint32_t result = NO_CFG;

    // Read all data from EEPROM
    readAll();

    // Initialize default values
    initDefault();

    if (eeprom.data.present == PRESENT)
    {
        if (eeprom.data.version == VERSION)
        {
            result = CFG_OK;
        }
        else if (eeprom.data.version < VERSION)
        {
            result = legacyImport();
        }
        else
        {
            result = CFG_UNKNOWN;
        }
    }
    else
    {
        //uint32_t error = EEPROMMassErase();
        //if (error)
        {
            System::error();
        }
    }

    // If no valid config could be found, initialize everything to
    // default values.
    if (result != CFG_OK && result != CFG_UPGRADED)
    {
        eeprom.data = defaultSettings;
        if (result == NO_CFG)
        {
            result = CFG_OK;
        }
    }

    /*
     * Let all other classes point to the memory here
     */
    uint32_t eepromProg = 0;
    uint32_t volatileProg = 0;
    for (uint32_t prog = 0; prog < MIDI::MAX_PROGRAMS; prog++)
    {
        if (volatileProg < ENV_PROG_OFFSET || eepromProg >= ENV_PROG_COUNT)
        {
            MIDI::programs[prog].steps = &(volatileData.envelopes[volatileProg++]);
        }
        else
        {
            MIDI::programs[prog].steps = &(eeprom.data.envelopes[eepromProg++]);
        }
    }
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        Coil::allCoils[coil].maxDutyPerm   = &(coilData[coil].maxDutyPerm);
        Coil::allCoils[coil].maxOntimeUS   = &(coilData[coil].maxOntimeUS);
        Coil::allCoils[coil].minOntimeUS   = &(coilData[coil].minOntimeUS);
        Coil::allCoils[coil].minOfftimeUS  = &(coilData[coil].minOfftimeUS);
        Coil::allCoils[coil].midi.coilMaxVoices = &(coilData[coil].midiMaxVoices);
        Coil::allCoils[coil].simple.filteredFrequency.factor   = &(coilData[coil].simpleBPSFF);
        Coil::allCoils[coil].simple.filteredFrequency.constant = &(coilData[coil].simpleBPSFC);
        Coil::allCoils[coil].simple.filteredOntimeUS.factor    = &(coilData[coil].simpleOntimeFF);
        Coil::allCoils[coil].simple.filteredOntimeUS.constant  = &(coilData[coil].simpleOntimeFC);
    }

    return result;
}

uint32_t EEPROMSettings::legacyImport()
{
    // v2 used 2 banks for wear leveling.
    uint8_t  present0 = eeprom.legacyV2.bank0.data.present;
    uint8_t  present1 = eeprom.legacyV2.bank1.data.present;
    uint8_t  version0 = eeprom.legacyV2.bank0.data.version;
    uint8_t  version1 = eeprom.legacyV2.bank1.data.version;
    uint16_t wear0    = eeprom.legacyV2.bank0.data.wear;
    uint16_t wear1    = eeprom.legacyV2.bank1.data.wear;
    if ((present0 == PRESENT && version0 == 0x02) || (present1 == PRESENT && version1 == 0x02))
    {
        // Found old v2 layout. Import to current layout.

        // A temporary storage needs to be created since both the legacy
        // and the current layout occupy the same memory area (thus direct
        // assignments from one to the other would corrupt everything).
        LegacyV2Bank oldLayout;

        if (present0 != PRESENT)
        {
            // No valid bank. Set wear to 0 such that it will be ignored
            wear0 = 0;
        }
        if (present1 != PRESENT)
        {
            wear1 = 0;
        }
        if (wear0 == 0 && wear1 == 0)
        {
            // No config present.
            return NO_CFG;
        }
        else if (wear0 > wear1)
        {
            oldLayout = eeprom.legacyV2.bank0;
        }
        else
        {
            oldLayout = eeprom.legacyV2.bank1;
        }

        /*
         * Since the old data has been backed up into oldLayout, the current
         * layout can now be initialized with the default values. Afterwards,
         * the old settings are copied over. This automatically sets all new
         * values to their default that may not be contained in the old layout.
         */
        eeprom.data = defaultSettings;

        // User data
        for (uint32_t user = 0; user < 3; user++)
        {
            UserData& userData = eeprom.data.userData[user];

            uint32_t copySize = std::min(V2_STR_CHAR_COUNT, STR_CHAR_COUNT);
            memset(userData.name,     '\x00', sizeof(userData.name));
            memset(userData.password, '\x00', sizeof(userData.password));

            memcpy(userData.name,     oldLayout.data.userNames[user], copySize);
            memcpy(userData.password, oldLayout.data.userPwds[user],  copySize);

            userData.maxDutyPerm =  (oldLayout.data.userSettings[user] & 0xff800000) >> 23;
            userData.maxBPS      = ((oldLayout.data.userSettings[user] & 0x007ff000) >> 12) * 10;
            userData.maxOntimeUS =  (oldLayout.data.userSettings[user] & 0x00000fff) * 10;
        }

        // Coil data
        for (uint32_t coil = 0; coil < 6; coil++)
        {
            CoilData& coilData = eeprom.data.coilData[coil];

            coilData.maxDutyPerm    =  (oldLayout.data.coilSettings[coil] & 0xff800000) >> 23;
            coilData.minOfftimeUS   = ((oldLayout.data.coilSettings[coil] & 0x007f0000) >> 16) * 10;
            coilData.midiMaxVoices  = ((oldLayout.data.coilSettings[coil] & 0x0000f000) >> 12) +  1;
            coilData.maxOntimeUS    =  (oldLayout.data.coilSettings[coil] & 0x00000fff)        * 10;
        }

        // Envelopes
        constexpr uint32_t ENV_AMP  = 0;
        constexpr uint32_t ENV_DUR  = 1;
        constexpr uint32_t ENV_NTAU = 2;
        constexpr uint32_t ENV_NEXT = 3;
        if (MIDIProgram::DATA_POINTS != V2_ENV_DATA_POINTS)
        {
            /*
             * Restore old layout just in case...
             * well, technically, only the valid bank will be restored but
             * that should be 100% compatible with the previous firmwares.
             */
            eeprom.legacyV2.bank0.data.present = present0;
            eeprom.legacyV2.bank0.data.version = version0;
            eeprom.legacyV2.bank0.data.wear    = wear0;
            eeprom.legacyV2.bank1.data.present = present1;
            eeprom.legacyV2.bank1.data.version = version1;
            eeprom.legacyV2.bank1.data.wear    = wear1;
            if (wear0 > wear1)
            {
                eeprom.legacyV2.bank0 = oldLayout;
            }
            else
            {
                eeprom.legacyV2.bank1 = oldLayout;
            }
            return CFG_UNKNOWN;
        }
        uint32_t maxImportProgs = std::min(ENV_PROG_COUNT, V2_ENV_PROG_COUNT);
        for (uint32_t prog = 0; prog < maxImportProgs; prog++)
        {
            for (uint32_t step = 0; step < V2_ENV_DATA_POINTS; step++)
            {
                MIDIProgram::DataPoint& dataPoint = eeprom.data.envelopes[prog][step];
                dataPoint.amplitude  = oldLayout.data.envelopes[prog][step][ENV_AMP].f32;
                dataPoint.durationUS = oldLayout.data.envelopes[prog][step][ENV_DUR].f32;
                dataPoint.ntau       = oldLayout.data.envelopes[prog][step][ENV_NTAU].f32;
                dataPoint.nextStep   = oldLayout.data.envelopes[prog][step][ENV_NEXT].ui32;
            }
        }

        // Other settings
        eeprom.data.deviceData.uiButtonHoldTime =  oldLayout.data.otherSettings[0]        & 0xffff;
        eeprom.data.deviceData.uiSleepDelay     = (oldLayout.data.otherSettings[0] >> 16) & 0xffff;
        eeprom.data.deviceData.uiBrightness     =  oldLayout.data.otherSettings[1]        & 0xff;
        eeprom.data.deviceData.uiColorMode      = (oldLayout.data.otherSettings[1] >>  9) & 0b1;

        // Import done
        eeprom.data.version = VERSION;
        return CFG_UPGRADED;
    }

    // Unknown config version; can't import.
    return CFG_UNKNOWN;
}

void EEPROMSettings::readAll()
{
    rwuAll(READ_EEPROM);
}

void EEPROMSettings::writeAll()
{
    rwuAll(WRITE_EEPROM);
}

void EEPROMSettings::updateAll()
{
    rwuAll(UPDATE_EEPROM);
}

void EEPROMSettings::rwuAll(uint32_t mode)
{
    byteAddress = 0;
    for (uint32_t page = 0; page < PAGE_COUNT; page++)
    {
        rwuSingle(mode, eeprom.raw[page], PAGE_SIZE);
    }
}

bool EEPROMSettings::rwuSingle(uint32_t mode, void *newData, uint32_t byteSize)
{
    // Returns if EEPROM has been modified.

    if (mode == READ_EEPROM)
    {
        readSequence(newData, byteSize);
        return false;
    }
    else if (mode == WRITE_EEPROM)
    {
        writeSequence(newData, byteSize);
        return true;
    }
    else if (mode == UPDATE_EEPROM)
    {
        return writeChangedSequence(newData, byteSize);
    }

    return false;
}

void EEPROMSettings::readSequence(void *newData, uint32_t byteSize)
{
    // Make sure byteSize is a multiple of 4 (= kill lowest two bits)
    byteSize &= ~0b11;
    EEPROMRead((uint32_t *)newData, byteAddress, byteSize);
    byteAddress += byteSize;
}

void EEPROMSettings::writeSequence(void *newData, uint32_t byteSize)
{
    // Make sure byteSize is a multiple of 4 (= kill lowest two bits)

    byteSize &= ~0b11;
    uint32_t error = EEPROMProgram((uint32_t *)newData, byteAddress, byteSize);
    if (error)
    {
        System::error();
    }
    byteAddress += byteSize;
}

bool EEPROMSettings::writeChangedSequence(void *newData, uint32_t byteSize)
{
    // Checks if data has changed and only then updates the EEPROM.
    // Takes a bit more time but EEPROM operations are slow anyway.
    // Returns if EEPROM has been modified.

    // Make sure byteSize is a multiple of 4 (= kill lowest two bits)
    byteSize &= ~0b11;

    /*
     *  Erases are done on a block base (1 block = 16 words, 1 word = 4 bytes).
     *  So if one word in a block needs to be changed, the whole block is
     *  erased (and rewritten). Since our data is likely smaller than 1 block
     *  we can abort the check on the first mismatch. In that case the whole
     *  block will be rewritten anyways.
     */
    bool dataChanged = false;
    uint32_t writeStartAddress = byteAddress;
    uint32_t temp = 0;
    uint32_t i;
    for (i = 0; i < (byteSize / 4); i++)
    {
        readSequence(&temp, 4);
        if (((uint32_t *)newData)[i] != temp)
        {
            dataChanged = true;
            break;
        }
    }
    if (dataChanged)
    {
        byteAddress = writeStartAddress;
        writeSequence(newData, byteSize);
    }
    return dataChanged;
}

void EEPROMSettings::initDefault()
{
    /*
     * Default "Header"
     */
    defaultSettings.present  = PRESENT;
    defaultSettings.version  = VERSION;
    defaultSettings.reserved = 1;

    /*
     * Default Coil Settings
     */
    for (uint32_t coil = 0; coil < 6; coil++)
    {
        defaultSettings.coilData[coil].maxDutyPerm    = 50;
        defaultSettings.coilData[coil].midiMaxVoices  =  8;
        defaultSettings.coilData[coil].maxOntimeUS    = 10;
        defaultSettings.coilData[coil].minOntimeUS    =  0;
        defaultSettings.coilData[coil].minOfftimeUS   = 10;
        defaultSettings.coilData[coil].outputInvert   = false;
        defaultSettings.coilData[coil].simpleBPSFC    = 5.0f;
        defaultSettings.coilData[coil].simpleBPSFF    = 1.8f;
        defaultSettings.coilData[coil].simpleOntimeFC = 30.0f;
        defaultSettings.coilData[coil].simpleOntimeFF = 2.0f;
    }

    /*
     * Default User Settings
     */
    defaultSettings.userData[0].maxBPS      = 100;
    defaultSettings.userData[0].maxDutyPerm = 10;
    defaultSettings.userData[0].maxOntimeUS = 10;
    strcpy(defaultSettings.userData[0].name, "Padawan");
    strcpy(defaultSettings.userData[0].password, "1234");
    defaultSettings.userData[1].maxBPS      = 200;
    defaultSettings.userData[1].maxDutyPerm = 20;
    defaultSettings.userData[1].maxOntimeUS = 20;
    strcpy(defaultSettings.userData[1].name, "Jedi Knight");
    strcpy(defaultSettings.userData[1].password, "8079");
    defaultSettings.userData[2].maxBPS      = 500;
    defaultSettings.userData[2].maxDutyPerm = 50;
    defaultSettings.userData[2].maxOntimeUS = 50;
    strcpy(defaultSettings.userData[2].name, "Master Yoda");
    strcpy(defaultSettings.userData[2].password, "0");

    /*
     * Default Device Settings
     */
    defaultSettings.deviceData.deviceID         = 0;
    defaultSettings.deviceData.eepromUpdateMode = 0; // Manual
    defaultSettings.deviceData.uiBackOff        = true;
    defaultSettings.deviceData.uiBrightness     = 100;
    defaultSettings.deviceData.uiButtonHoldTime = 250;
    defaultSettings.deviceData.uiColorMode      = 1; // Dark mode
    defaultSettings.deviceData.uiSleepDelay     = 0; // No sleep
    defaultSettings.deviceData.midiLfoPeriodUS  = 1.0f / 5.0f; // 5Hz

    /*
     * Default Envelopes
     */
    constexpr uint8_t LAST = MIDIProgram::DATA_POINTS - 1;

    // Program 0 and all unspecified programs: No envelope (constant 100% volume while on, no rise/fall times)
    for (uint8_t step = 0; step <= LAST; step++)
    {
        volatileData.envelopes[0][step] = {.amplitude = 1.0f, .durationUS = 1.0f, .ntau = 0.1f, .nextStep = (uint8_t) (step + 1)};
    }
    volatileData.envelopes[0][LAST - 1].nextStep = LAST - 1;
    volatileData.envelopes[0][LAST].amplitude    = 0.0f;
    volatileData.envelopes[0][LAST].nextStep     = LAST;

    // Initialize all EEPROM programs to the "empty" envelope
    for (uint32_t env = 0; env < ENV_PROG_COUNT; env++)
    {
        memcpy(defaultSettings.envelopes[env], volatileData.envelopes[0], sizeof(defaultSettings.envelopes[0]));
    }

    // Initialize all RAM/Volatile programs to the empty envelope, too, then set individual datapoints.
    for (uint32_t env = 1; env < (MIDI::MAX_PROGRAMS - ENV_PROG_COUNT); env++)
    {
        memcpy(volatileData.envelopes[env], volatileData.envelopes[0], sizeof(volatileData.envelopes[0]));
    }

    // Note: these programs date back to the early days of Syntherrupter where
    //       envelopes were always fixed 4 steps. While not every envelope requires
    //       4 explicitly defined steps, it was easier to keep it this way.

    // Program 1: Linear Piano envelope, peaking to 1
    volatileData.envelopes[1][0]    = {.amplitude =  1.0f, .durationUS =     30000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[1][1]    = {.amplitude =  0.5f, .durationUS =     10000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[1][2]    = {.amplitude =  0.1f, .durationUS =   3500000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[1][LAST] = {.amplitude =  0.0f, .durationUS =     10000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 2: Slow Pad Sound (very slow rise and fall times)
    volatileData.envelopes[2][0]    = {.amplitude =  1.0f, .durationUS =   4000000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[2][1]    = {.amplitude =  1.0f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[2][2]    = {.amplitude =  1.0f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[2][LAST] = {.amplitude =  0.0f, .durationUS =   1000000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 3: Slow Step Pad (like program 2 but with a step at the beginning, such that it
    volatileData.envelopes[3][0]    = {.amplitude =  0.3f, .durationUS =      8000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[3][1]    = {.amplitude =  1.0f, .durationUS =   4000000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[3][2]    = {.amplitude =  1.0f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[3][LAST] = {.amplitude =  0.0f, .durationUS =   1000000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 4: Pad (slow rise and fall times)
    volatileData.envelopes[4][0]    = {.amplitude =  1.0f, .durationUS =   1500000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[4][1]    = {.amplitude =  1.0f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[4][2]    = {.amplitude = 1.00f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[4][LAST] = {.amplitude =  0.0f, .durationUS =    500000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 5: Staccato, no reverb, peak amp = 1
    volatileData.envelopes[5][0]    = {.amplitude =  1.0f, .durationUS =      3000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[5][1]    = {.amplitude =  0.4f, .durationUS =     30000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[5][2]    = {.amplitude = 0.00f, .durationUS =    400000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[5][LAST] = {.amplitude =  0.0f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 6: "Legato", like program 1 with very slow release
    volatileData.envelopes[6][0]    = {.amplitude =  1.0f, .durationUS =      7000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[6][1]    = {.amplitude =  0.5f, .durationUS =     10000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[6][2]    = {.amplitude = 0.25f, .durationUS =   3000000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[6][LAST] = {.amplitude =  0.0f, .durationUS =   3000000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 7: Slow Step Pad (like program 2) with faster release
    volatileData.envelopes[7][0]    = {.amplitude =  0.3f, .durationUS =      8000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[7][1]    = {.amplitude =  1.0f, .durationUS =   4000000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[7][2]    = {.amplitude = 1.00f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[7][LAST] = {.amplitude =  0.0f, .durationUS =    400000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 8: Linear Piano with peak amp = 2.0f
    volatileData.envelopes[8][0]    = {.amplitude =  2.0f, .durationUS =     30000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[8][1]    = {.amplitude =  1.0f, .durationUS =      2500.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[8][2]    = {.amplitude = 0.10f, .durationUS =   3500000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[8][LAST] = {.amplitude =  0.0f, .durationUS =     10000.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 9: Staccato with reverb and peak amp = 3.0f
    volatileData.envelopes[9][0]    = {.amplitude =  3.0f, .durationUS =      3000.0f, .ntau = 0.1f, .nextStep = 1};
    volatileData.envelopes[9][1]    = {.amplitude =  1.0f, .durationUS =     27000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[9][2]    = {.amplitude = 0.00f, .durationUS =    400000.0f, .ntau = 0.1f, .nextStep = 2};
    volatileData.envelopes[9][LAST] = {.amplitude =  0.0f, .durationUS =         1.0f, .ntau = 0.1f, .nextStep = LAST};

    // Program 10: "True" Piano Envelope (using exp curves)
    volatileData.envelopes[10][0]    = {.amplitude = 2.0f, .durationUS =        15e3f, .ntau = 2.0f, .nextStep = 1};
    volatileData.envelopes[10][1]    = {.amplitude = 1.0f, .durationUS =       100e3f, .ntau = 2.0f, .nextStep = 2};
    volatileData.envelopes[10][2]    = {.amplitude = 0.0f, .durationUS =      8000e3f, .ntau = 7.0f, .nextStep = 2};
    volatileData.envelopes[10][LAST] = {.amplitude = 0.0f, .durationUS =       200e3f, .ntau = 4.0f, .nextStep = LAST};

    // Program 11-19: Same as 1-9 but with exp curves
    for (uint32_t prog = 1; prog <= 9; prog++)
    {
        memcpy(volatileData.envelopes[prog + 10], volatileData.envelopes[prog], sizeof(volatileData.envelopes[0]));
        for (uint32_t step = 0; step < MIDIProgram::DATA_POINTS; step++)
        {
            volatileData.envelopes[prog + 10][step].ntau = 3.0f;
        }
    }
}
