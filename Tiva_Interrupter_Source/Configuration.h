/*
 * EEPROM.h
 *
 *  Created on: 28.04.2020
 *      Author: Max Zuidberg
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_


#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/eeprom.h"
#include "System.h"


class Configuration
{
public:
    Configuration();
    virtual ~Configuration();
    bool init(System *sys);
    void read();
    void write();
    void update();
    uint32_t getUsersMaxOntimeUS(uint32_t user);
    uint32_t getUsersMaxBPS(uint32_t user);
    uint32_t getUsersMaxDutyPerm(uint32_t user);
    uint32_t getCoilsMaxOntimeUS(uint32_t coil);
    uint32_t getCoilsMaxBPS(uint32_t coil);
    uint32_t getCoilsMaxDutyPerm(uint32_t coil);
    uint32_t getStngsHoldTime();
    uint32_t getStngsBrightness();
    static constexpr uint32_t STR_CHAR_COUNT = 32; // must be multiple of 4
    // +1 because strings are null terminated. This Null character won't take any space in the EEPROM.
    char userNames[3][STR_CHAR_COUNT + 1] = {"Padawan", "Jedi Knight", "Master Yoda"};
    char userPwds[3][STR_CHAR_COUNT + 1]  = {"1234",    "8079",        "2813"};
    uint32_t userSettings[3]              = {0, 0, 0};          // Bit format equal to communication format, documented in separate file.
    uint32_t coilSettings[6]              = {0, 0, 0, 0, 0, 0}; // Bit format equal to communication format, documented in separate file.
    uint32_t otherSettings[10];  // Bit format equal to communication format, documented in separate file.
private:
    void rwuAll(uint32_t mode);
    bool rwuSingle(uint32_t mode, void *newData, uint32_t byteSize);
    void readSequence(void *newData, uint32_t byteSize);
    void writeSequence(void *newData, uint32_t byteSize);
    bool writeChangedSequence(void *newData, uint32_t byteSize);
    bool updateBank();

    static constexpr uint32_t CFG_PRESENT = 0x42000000; // MSB: Random value != 0 and != 0xff to check if data has been written to the EEPROM. Next byte: Config Version. Increments if there has been incompatible changes. Lowest two bytes: wear leveling. Switch to next bank on overflow.
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

#endif /* CONFIGURATION_H_ */
