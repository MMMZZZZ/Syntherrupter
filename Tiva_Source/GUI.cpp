/*
 * GUI.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max
 */

#include <GUI.h>

GUI::GUI()
{
    // TODO Auto-generated constructor stub

}

GUI::~GUI()
{
    // TODO Auto-generated destructor stub
}

void GUI::midiUartISR()
{
    guiMidi.uartISR();
}

void GUI::init(System* sys, void (*midiISR)(void))
{
    guiSys = sys;
    guiNxt.init(guiSys, 3, 57600);
    guiMidi.init(guiSys, 0, 115200, midiISR);
    guiOut.init(guiSys);
    guiFilteredFrequency.init(guiSys, 0.01f, 10000);
    guiFilteredOntimeUS.init(guiSys, 0.05f, 10000);

    while (guiNxt.getVal("comOk") != 1)
    {
        guiNxt.sendCmd("rest");
        guiSys->delayUS(1000000);
        guiNxt.setVal("comOk", 1);
        guiSys->delayUS(5000);
    }
    guiNxt.setVal("comOk", 2);
}

void GUI::setError(const char* err)
{
    bool endOfErr = false;
    for (uint32_t i = 0; i < guiErrorLen; i++)
    {
        if (!endOfErr)
        {
            guiErrorTxt[i] = err[i];
            endOfErr = (err[i] == '\0');
        }
        else
        {
            guiErrorTxt[i] = '\0';
        }
    }
}

void GUI::showError()
{
    guiNxt.setPage("Error");
    guiNxt.setTxt("tInfo", guiErrorTxt);
}


bool GUI::checkValue(uint32_t val)
{
    if (val == guiNxt.receiveErrorVal)
    {
        setError("Wert unplausibel");
        return false;
    }
    if (val == guiNxt.receiveTimeoutVal)
    {
        setError("Zeitüberschreitung");
        return false;
    }
    return true;
}

bool GUI::update()
{
    if (guiNxt.charsAvail())
    {
        char mode = guiNxt.getChar();

        switch (mode)
        {
        case 'm':
            if (!guiMidi.isEnabled())
            {
                guiMidi.enable();
            }
            guiMidi.play();
            break;
        case 'X':
            setError("Fehler von Nextion");
            return false;
        case 'w':
            guiMidi.stop();
            guiOntimeUS = 0;
            guiPeriodUS = 0;
            break;
        case 'u':
            guiMidi.disable();
            guiMaxDutyPerc = guiNxt.getVal("Settings.maxDuty");
            if (!checkValue(guiMaxDutyPerc))
            {
                return false;
            }
            guiMaxOntimeUS = guiNxt.getVal("Settings.maxOnTime");
            if (!checkValue(guiMaxOntimeUS))
            {
                return false;
            }
            guiOut.setMaxOntimeUS(guiMaxOntimeUS);
            guiOut.setMaxDutyPerc(guiMaxDutyPerc);
            guiNxt.setPage("Menu");
            guiNxt.setVal("MIDILive.sVol", 0);
            guiNxt.setVal("MIDILive.nVol", 0);
            break;
        case 's':
            guiMidi.disable();
            guiOntimeUS = guiNxt.getVal("oldOn");
            if (!checkValue(guiOntimeUS))
            {
                return false;
            }
            guiPeriodUS = guiNxt.getVal("oldPer");
            if (!checkValue(guiPeriodUS))
            {
                return false;
            }

            break;
        case 'v':
            if (guiMidi.isEnabled())
            {
                uint32_t vol = guiNxt.getVal("nVol");
                if (!checkValue(vol))
                {
                    return false;
                }
                uint32_t volMode = guiNxt.getVal("Settings.volMode");
                if (volMode == guiMidi.MIDI_VOL_RELATIVE)
                {
                    guiMidi.setVolumeMode(volMode);
                    guiMidi.setVolumeMax((vol * guiMaxDutyPerc) / 100.0f);
                }
                else if (volMode == guiMidi.MIDI_VOL_ABSOLUTE)
                {
                    guiMidi.setVolumeMode(volMode);
                    guiMidi.setVolumeMax(vol);
                }
                else
                {
                    setError("volMode unbekannt");
                    return false;
                }
                guiMidi.play();
            }
            break;
        default:
            guiNxt.flushRx();
            break;
        }
        guiFilteredOntimeUS.setTarget(guiOntimeUS);
        if (guiPeriodUS)
        {
            guiFilteredFrequency.setTarget(1000000.0f / guiPeriodUS);
        }
        else
        {
            guiFilteredFrequency.setTarget(0);
        }
    }
    return true;
}

void GUI::applyOutput()
{
    if (guiMidi.isEnabled())
    {
        guiMidi.updateFrequencyOntime();
        float f = guiMidi.getFrequency();
        float o = guiMidi.getOntimeUS();
        guiOut.tone(f, o);
    }
    else
    {
        float f = guiFilteredFrequency.getFiltered();
        float o = guiFilteredOntimeUS.getFiltered();
        guiOut.tone(f, o);
    }
}
