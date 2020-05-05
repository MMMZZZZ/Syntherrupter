/*
 * GUI.h
 *
 *  Created on: 26.03.2020
 *      Author: Max
 */

#ifndef GUI_H_
#define GUI_H_


#include "stdint.h"
#include "stdbool.h"
#include "driverlib/uart.h"
#include "System.h"
#include "Nextion.h"
#include "Coil.h"
#include "Configuration.h"

// Modes
enum Mode {
    exit,
    idle,
    simple,
    midiLive,
    userSelect,
    settings,
    nxtFWUpdate,
};


class GUI
{
public:
    GUI();
    virtual ~GUI();
    void init(System* sys, void (*midiISR)(void));
    bool update();
    void applyOutput();
    void setError(const char* err);
    void showError();
    void midiUartISR();
private:
    bool checkValue(uint32_t val);

    System* guiSys;
    Nextion guiNxt;
    Configuration guiCfg;
    static constexpr uint32_t guiCoilCount = 1;
    Coil guiCoils[guiCoilCount];

    uint32_t guiUserMaxOntimeUS = 0;
    uint32_t guiUserMaxBPS = 0;
    uint32_t guiUserMaxDutyPerm = 0;
    uint32_t guiCommand = 0;
    uint32_t guiCommandData[33] = {0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0, 0}; // sized to max length of all commands.
    Mode guiMode = idle;

    static constexpr uint32_t guiNxtTimeoutUS = 300000;
    static constexpr uint32_t guiErrorLen = 20;
    char guiErrorTxt[guiErrorLen];
};

#endif /* GUI_H_ */
