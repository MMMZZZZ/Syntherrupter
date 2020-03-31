/*
 * GUI.h
 *
 *  Created on: 26.03.2020
 *      Author: Max
 */

#ifndef GUI_H_
#define GUI_H_

#include "Filter.h"
#include "System.h"
#include "Nextion.h"
#include "MIDI.h"
#include "Output.h"


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
    MIDI guiMidi;
    Output guiOut;
    Filter guiFilteredOntimeUS;
    Filter guiFilteredFrequency;

    uint32_t guiMaxDutyPerc = 1;
    uint32_t guiMaxOntimeUS = 1;
    uint32_t guiOntimeUS = 0;
    uint32_t guiPeriodUS = 0;

    const uint32_t guiErrorLen = 20;
    char guiErrorTxt[20];
};

#endif /* GUI_H_ */
