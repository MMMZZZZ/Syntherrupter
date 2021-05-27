/*
 * EEPROMSettings.cpp
 *
 *  Created on: 28.04.2020
 *      Author: Max Zuidberg
 */

#include <EEPROMSettings.h>


constexpr uint32_t EEPROMSettings::BANK_STARTS[BANK_COUNT];
char     EEPROMSettings::userNames[3][STR_CHAR_COUNT] = {"Padawan", "Jedi Knight", "Master Yoda"};
char     EEPROMSettings::userPwds[3][STR_CHAR_COUNT]  = {"1234",    "8079",        "0"};
uint32_t EEPROMSettings::userSettings[3]          = {0, 0, 0};          // Bit format equal to communication format, documented in separate file.
uint32_t EEPROMSettings::coilSettings[6]          = {0, 0, 0, 0, 0, 0}; // Bit format equal to communication format, documented in separate file.
uint32_t EEPROMSettings::otherSettings[10]        = {((0 << 16) | 250), 100, 0, 0, 0,
                                                     0, 0, 0, 0, 0};  // Bit format equal to communication format, documented in separate file.
uint32_t EEPROMSettings::EnvelopeSettings[ENV_PROG_COUNT][MIDIProgram::DATA_POINTS][4];
uint32_t EEPROMSettings::byteAddress = 0;
uint32_t EEPROMSettings::bank = BANK_COUNT; // initialized to value higher than normally possible
bool     EEPROMSettings::EEPROMUpToDate = false;
EEPROMSettings::EEPROMData EEPROMSettings::eeprom;
EEPROMSettings::UserData (&EEPROMSettings::userData)[3] = EEPROMSettings::eeprom.parameters.userData;
EEPROMSettings::CoilData (&EEPROMSettings::coilData)[6] = EEPROMSettings::eeprom.parameters.coilData;
EEPROMSettings::UIData &(EEPROMSettings::uiData) = EEPROMSettings::eeprom.parameters.uiData;


bool EEPROMSettings::init()
{
    /*
     * Total Configuration cannot be larger than 2040 Bytes.
     * Reason: This class divides the 6kB EEPROM in 3 banks. It switches
     * automatically between them if a certain number of writes has been made.
     * This extends the nominal life of 500k writes (should actually be 700k,
     * based on how it internally works) to at least 1.5M writes. So you could
     * change a value every minute for almost three years.
     */

    // Make EEPROM available and initialize
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    SysCtlDelay(2);
    if (EEPROMInit() != EEPROM_INIT_OK)
    {
        // Power supply issues or EEPROM damaged / end of life
        System::error();
    }

    // Search for a bank with valid data and return if such data could be found.
    bool foundValidConfig = updateBank();

    if (!foundValidConfig)
    {
        uint32_t error = EEPROMMassErase();
        if (error)
        {
            System::error();
        }
        updateBank();
    }
    else
    {
        read();
    }

    // Let all other classes point to the memory here
    for (uint32_t program = 0; program < MIDI::MAX_PROGRAMS; program++)
    {
        MIDI::programs[program].stepsPointer = &(eeprom.parameters.envelopes[program]);
    }
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        Coil::allCoils[coil].maxDutyPermPtr  = &(coilData[coil].maxDutyPerm);
        Coil::allCoils[coil].maxOntimeUSPtr  = &(coilData[coil].maxOntimeUS);
        Coil::allCoils[coil].minOntimeUSPtr  = &(coilData[coil].minOntimeUS);
        Coil::allCoils[coil].minOfftimeUSPtr = &(coilData[coil].minOfftimeUS);
        Coil::allCoils[coil].midi.coilMaxVoicesPtr = &(coilData[coil].maxMidiVoices);
        Coil::allCoils[coil].simple.filteredFrequency.factorPtr   = &(coilData[coil].simpleBPSFF);
        Coil::allCoils[coil].simple.filteredFrequency.constantPtr = &(coilData[coil].simpleBPSFC);
        Coil::allCoils[coil].simple.filteredOntimeUS.factorPtr    = &(coilData[coil].simpleOntimeFF);
        Coil::allCoils[coil].simple.filteredOntimeUS.constantPtr  = &(coilData[coil].simpleOntimeFC);
    }

    return foundValidConfig;
}

uint32_t EEPROMSettings::getUsersMaxDutyPerm(uint32_t user)
{
    uint32_t duty = 0;
    if (user < 3)
    {
        duty = (userSettings[user] & 0xff800000) >> 23;
    }
    return duty;
}

uint32_t EEPROMSettings::getUsersMaxBPS(uint32_t user)
{
    uint32_t bps = 0;
    if (user < 3)
    {
        bps = ((userSettings[user] & 0x007ff000) >> 12) * 10;
    }
    return bps;
}

uint32_t EEPROMSettings::getUsersMaxOntimeUS(uint32_t user)
{
    uint32_t ontimeUS = 0;
    if (user < 3)
    {
        ontimeUS = (userSettings[user] & 0x00000fff) * 10;
    }
    return ontimeUS;
}

uint32_t EEPROMSettings::getCoilsMaxDutyPerm(uint32_t coil)
{
    uint32_t duty = 0;
    if (coil < 6)
    {
        duty = (coilSettings[coil] & 0xff800000) >> 23;
    }
    return duty;
}

uint32_t EEPROMSettings::getCoilsMinOffUS(uint32_t coil)
{
    uint32_t minOffUS = 0;
    if (coil < 6)
    {
        minOffUS = ((coilSettings[coil] & 0x007f0000) >> 16) * 10;
    }
    return minOffUS;
}

uint32_t EEPROMSettings::getCoilsMaxVoices(uint32_t coil)
{
    uint32_t maxVoices = 0;
    if (coil < 6)
    {
        maxVoices = ((coilSettings[coil] & 0x0000f000) >> 12) + 1;
    }
    return maxVoices;
}

uint32_t EEPROMSettings::getCoilsMaxOntimeUS(uint32_t coil)
{
    uint32_t ontimeUS = 0;
    if (coil < 6)
    {
        ontimeUS = (coilSettings[coil] & 0x00000fff) * 10;
    }
    return ontimeUS;
}

void EEPROMSettings::getMIDIPrograms()
{
    for (uint32_t program = 0; program < ENV_PROG_COUNT; program++)
    {
        for (uint32_t dataPoint = 0; dataPoint < MIDIProgram::DATA_POINTS; dataPoint++)
        {
            float amplitude  = ANY_TO_FLOAT(EnvelopeSettings[program][dataPoint][ENV_AMP]);
            float durationUS = ANY_TO_FLOAT(EnvelopeSettings[program][dataPoint][ENV_DUR]);
            float ntau       = ANY_TO_FLOAT(EnvelopeSettings[program][dataPoint][ENV_NTAU]);
            float nextStep   =              EnvelopeSettings[program][dataPoint][ENV_NEXT];

            MIDI::programs[ENV_PROG_OFFSET + program].setDataPoint(dataPoint, amplitude, durationUS, ntau, nextStep);
        }
    }
}

void EEPROMSettings::setMIDIPrograms()
{
    for (uint32_t program = 0; program < ENV_PROG_COUNT; program++)
    {
        for (uint32_t dataPoint = 0; dataPoint < MIDIProgram::DATA_POINTS; dataPoint++)
        {
            EnvelopeSettings[program][dataPoint][ENV_AMP]  = ANY_TO_UINT32(MIDI::programs[ENV_DUR + program].steps[dataPoint].amplitude);
            EnvelopeSettings[program][dataPoint][ENV_DUR]  = ANY_TO_UINT32(MIDI::programs[ENV_DUR + program].steps[dataPoint].durationUS);
            EnvelopeSettings[program][dataPoint][ENV_NTAU] = ANY_TO_UINT32(MIDI::programs[ENV_DUR + program].steps[dataPoint].ntau);
            EnvelopeSettings[program][dataPoint][ENV_NEXT] =               MIDI::programs[ENV_DUR + program].steps[dataPoint].nextStep;
        }
    }
}

bool EEPROMSettings::updateBank()
{
    /*
     * If no bank is selected, return value indicates if a valid configuration
     * could be found.
     * Else the return value indicates if we switched to a new bank.
     */
    if (bank >= BANK_COUNT)
    {
        // Currently no bank is selected. Search for valid bank.
        for (uint32_t i = 0; i < BANK_COUNT; i++)
        {
            uint32_t data = 0;
            EEPROMRead(&data, BANK_STARTS[i], 4); // @suppress("Invalid arguments")
            // Check if bank contains data and config version matches
            if ((data & 0xffff0000) == (PRESENT & 0xffff0000))
            {
                // Check if wear is non-zero, meaning the bank is in use.
                if (data & 0x0000ffff)
                {
                    bank = i;
                    return true;
                }
            }
        }
        return false;
    }
    else
    {
        // Check bank wear level and switch to next one if necessary
        uint32_t data = 0;
        EEPROMRead(&data, BANK_STARTS[bank], 4);
        data &= 0x0000ffff;
        if (data == 0xffff)
        {
            // Time to switch bank
            data = PRESENT;
            // Mark bank as unused
            EEPROMProgram(&data, BANK_STARTS[bank], 4);
            // Select and initialize next bank
            if (++bank >= BANK_COUNT)
            {
                bank = 0;
            }
            data = PRESENT + 1;
            EEPROMProgram(&data, BANK_STARTS[bank], 4);
            return true;
        }
    }
    return false;
}

void EEPROMSettings::read()
{
    rwuAll(READ_EEPROM);
}

void EEPROMSettings::write()
{
    rwuAll(WRITE_EEPROM);
}

void EEPROMSettings::update()
{
    // Needs more testing! Edit 19.09.20: Apparently it works fine.
    rwuAll(UPDATE_EEPROM);
}

void EEPROMSettings::rwuAll(uint32_t mode)
{
    for (uint32_t page = 0; page < PAGE_COUNT; page++)
    {
        rwuSingle(mode, eeprom.raw[page], PAGE_SIZE);
    }
}

void EEPROMSettings::rwuAllLegacyV2(uint32_t mode)
{

    if (bank >= BANK_COUNT)
    {
        // No data in EEPROM yet. Select and initialize bank 0
        bank = 0;
        if (mode != READ_EEPROM)
        {
            uint32_t data = PRESENT + 1;
            EEPROMProgram(&data, BANK_STARTS[0], 4); // @suppress("Invalid arguments")
        }
    }
    byteAddress = BANK_STARTS[bank] + 4;

    bool EEPROMModified = false;
    EEPROMModified |= rwuSingle(mode, userNames, sizeof(userNames));
    EEPROMModified |= rwuSingle(mode, userPwds,  sizeof(userPwds));
    EEPROMModified |= rwuSingle(mode, userSettings, sizeof(userSettings));
    EEPROMModified |= rwuSingle(mode, coilSettings, sizeof(coilSettings));
    EEPROMModified |= rwuSingle(mode, otherSettings, sizeof(coilSettings));

    for (uint32_t i = 0; i < ENV_PROG_COUNT; i+=2)
    {
        EEPROMModified |= rwuSingle(mode, EnvelopeSettings[i], 2 * sizeof(EnvelopeSettings[i]));
    }

    if (EEPROMModified)
    {
        // If data has been written to the EEPROM, check wear level.
        updateBank();
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
        byteAddress -= i * 4;
        writeSequence(newData, byteSize);
    }
    return dataChanged;
}
