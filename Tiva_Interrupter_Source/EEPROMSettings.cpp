/*
 * EEPROMSettings.cpp
 *
 *  Created on: 28.04.2020
 *      Author: Max
 */

#include <EEPROMSettings.h>

EEPROMSettings::EEPROMSettings()
{

}

EEPROMSettings::~EEPROMSettings()
{

}

bool EEPROMSettings::init(System *sys)
{
    /*
     * Total Configuration cannot be larger than 2040 Bytes.
     * Reason: This class divides the 6kB EEPROM in 3 banks. It switches
     * automatically between them if a certain number of writes has been made.
     * This extends the nominal life of 500k writes (should actually be 700k,
     * based on how it internally works) to at least 1.5M writes. So you could
     * change a value every minute for almost three years.
     */
    cfgSys = sys;

    // Make EEPROM available and initialize
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    SysCtlDelay(2);
    if (EEPROMInit() != EEPROM_INIT_OK)
    {
        // Power supply issues or EEPROM damaged / end of life
        cfgSys->error();
    }

    // Search for a bank with valid data and return if such data could be found.
    bool foundValidConfig = updateBank();

    if (!foundValidConfig)
    {
        uint32_t error = EEPROMMassErase();
        if (error)
        {
            cfgSys->error();
        }
        updateBank();
    }
    else
    {
        read();
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
        minOffUS = ((coilSettings[coil] & 0x007ff000) >> 12);
    }
    return minOffUS;
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

bool EEPROMSettings::updateBank()
{
    /*
     * If no bank is selected, return value indicates if a valid configuration
     * could be found.
     * Else the return value indicates if we switched to a new bank.
     */
    if (cfgBank >= CFG_BANK_COUNT)
    {
        // Currently no bank is selected. Search for valid bank.
        for (uint32_t i = 0; i < CFG_BANK_COUNT; i++)
        {
            uint32_t data = 0;
            EEPROMRead(&data, CFG_BANK_STARTS[i], 4);
            // Check if bank contains data and config version matches
            if ((data & 0xffff0000) == (CFG_PRESENT & 0xffff0000))
            {
                // Check if wear is non-zero, meaning the bank is in use.
                if (data & 0x0000ffff)
                {
                    cfgBank = i;
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
        EEPROMRead(&data, CFG_BANK_STARTS[cfgBank], 4);
        data &= 0x0000ffff;
        if (data == 0xffff)
        {
            // Time to switch bank
            data = CFG_PRESENT;
            // Mark bank as unused
            EEPROMProgram(&data, CFG_BANK_STARTS[cfgBank], 4);
            // Select and initialize next bank
            if (++cfgBank >= CFG_BANK_COUNT)
            {
                cfgBank = 0;
            }
            data = CFG_PRESENT + 1;
            EEPROMProgram(&data, CFG_BANK_STARTS[cfgBank], 4);
            return true;
        }
    }
    return false;
}

void EEPROMSettings::read()
{
    rwuAll(CFG_READ_EEPROM);
}

void EEPROMSettings::write()
{
    rwuAll(CFG_WRITE_EEPROM);
}

void EEPROMSettings::update()
{
    // TODO Needs more testing!
    rwuAll(CFG_UPDATE_EEPROM);
}

void EEPROMSettings::rwuAll(uint32_t mode)
{

    if (cfgBank >= CFG_BANK_COUNT)
    {
        // No data in EEPROM yet. Select and initialize bank 0
        cfgBank = 0;
        if (mode != CFG_READ_EEPROM)
        {
            uint32_t data = CFG_PRESENT + 1;
            EEPROMProgram(&data, CFG_BANK_STARTS[0], 4);
        }
    }
    cfgByteAddress = CFG_BANK_STARTS[cfgBank] + 4;

    bool EEPROMModified = false;
    EEPROMModified |= rwuSingle(mode, userNames, sizeof(userNames));
    EEPROMModified |= rwuSingle(mode, userPwds,  sizeof(userPwds));
    EEPROMModified |= rwuSingle(mode, userSettings, sizeof(userSettings));
    EEPROMModified |= rwuSingle(mode, coilSettings, sizeof(coilSettings));
    EEPROMModified |= rwuSingle(mode, otherSettings, sizeof(coilSettings));

    if (EEPROMModified)
    {
        // If data has been written to the EEPROM, check wear level.
        updateBank();
    }
}

bool EEPROMSettings::rwuSingle(uint32_t mode, void *newData, uint32_t byteSize)
{
    // Returns if EEPROM has been modified.

    if (mode == CFG_READ_EEPROM)
    {
        readSequence(newData, byteSize);
        return false;
    }
    else if (mode == CFG_WRITE_EEPROM)
    {
        writeSequence(newData, byteSize);
        return true;
    }
    else if (mode == CFG_UPDATE_EEPROM)
    {
        return writeChangedSequence(newData, byteSize);
    }

    return false;
}

void EEPROMSettings::readSequence(void *newData, uint32_t byteSize)
{
    // Make sure byteSize is a multiple of 4
    byteSize = byteSize >> 2;
    byteSize = byteSize << 2;
    EEPROMRead((uint32_t *)newData, cfgByteAddress, byteSize);
    cfgByteAddress += byteSize;
}

void EEPROMSettings::writeSequence(void *newData, uint32_t byteSize)
{
    // Make sure byteSize is a multiple of 4
    byteSize = byteSize >> 2;
    byteSize = byteSize << 2;
    uint32_t error = EEPROMProgram((uint32_t *)newData, cfgByteAddress, byteSize);
    if (error)
    {
        cfgSys->error();
    }
    cfgByteAddress += byteSize;
}

bool EEPROMSettings::writeChangedSequence(void *newData, uint32_t byteSize)
{
    // Checks if data has changed and only then updates the EEPROM.
    // Takes a bit more time but EEPROM operations are slow anyway.
    // Returns if EEPROM has been modified.

    // Make sure byteSize is a multiple of 4
    byteSize = byteSize >> 2;
    byteSize = byteSize << 2;

    /*
     *  Erases are done on a block base (1 block = 96 words, 1 word = 4 bytes).
     *  So if one word in a block needs to be changed, the whole block is
     *  erased (and rewritten). Since our data is likely smaller than 1 block
     *  we can abort the check on the first mismatch. In that case the whole
     *  block will be rewritten anyways.
     */
    bool dataChanged = false;
    readSequence(tempArray, byteSize);
    for (uint32_t i = 0; i < (byteSize / 4); i++)
    {
        if (((uint32_t *)newData)[i] != tempArray[i])
        {
            dataChanged = true;
            break;
        }
    }
    if (dataChanged)
    {
        cfgByteAddress -= byteSize;
        writeSequence(newData, byteSize);
    }
    return dataChanged;
}
