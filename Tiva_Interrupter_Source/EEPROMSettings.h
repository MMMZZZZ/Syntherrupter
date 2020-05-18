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


class EEPROMSettings
{
public:
    EEPROMSettings();
    virtual ~EEPROMSettings();
    bool init(System *sys);
    void read();
    void write();
    void update();
    uint32_t getUsersMaxOntimeUS(uint32_t user);
    uint32_t getUsersMaxBPS(uint32_t user);
    uint32_t getUsersMaxDutyPerm(uint32_t user);
    uint32_t getCoilsMaxOntimeUS(uint32_t coil);
    uint32_t getCoilsMinOffUS(uint32_t coil);
    uint32_t getCoilsMaxDutyPerm(uint32_t coil);
    uint32_t getStngsHoldTime();
    uint32_t getStngsBrightness();
    static constexpr uint32_t STR_CHAR_COUNT = 32; // must be multiple of 4
    char userNames[3][STR_CHAR_COUNT] = {"Padawan", "Jedi Knight", "Master Yoda"};
    char userPwds[3][STR_CHAR_COUNT]  = {"1234",    "8079",        "0"};
    uint32_t userSettings[3]              = {0, 0, 0};          // Bit format equal to communication format, documented in separate file.
    uint32_t coilSettings[6]              = {0, 0, 0, 0, 0, 0}; // Bit format equal to communication format, documented in separate file.
    uint32_t otherSettings[10]            = {((0 << 16) | 250), 100, 0, 0, 0,
                                             0, 0, 0, 0, 0};  // Bit format equal to communication format, documented in separate file.
private:
    void rwuAll(uint32_t mode);
    bool rwuSingle(uint32_t mode, void *newData, uint32_t byteSize);
    void readSequence(void *newData, uint32_t byteSize);
    void writeSequence(void *newData, uint32_t byteSize);
    bool writeChangedSequence(void *newData, uint32_t byteSize);
    bool updateBank();

    static constexpr uint32_t CFG_PRESENT = 0x42020000; // MSB: Random value != 0 and != 0xff to check if data has been written to the EEPROM. Next byte: Config Version. Increments if there has been incompatible changes. Lowest two bytes: wear leveling. Switch to next bank on overflow.
    static constexpr uint32_t CFG_WRITE_EEPROM = 0;
    static constexpr uint32_t CFG_READ_EEPROM = 1;
    static constexpr uint32_t CFG_UPDATE_EEPROM = 2;
    static constexpr uint32_t CFG_BANK_COUNT    = 3;
    const uint32_t CFG_BANK_STARTS[3] = {0, 2048, 4096};


    System *cfgSys;

    uint32_t cfgByteAddress = 0;
    uint32_t cfgBank = CFG_BANK_COUNT; // initialized to value higher than normally possible
    uint32_t tempArray[30]; // must be at least as large as the largest array in use.
    bool cfgEEPROMUpToDate = false;
};

#endif /* EEPROMSETTINGS_H_ */
