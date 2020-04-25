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
    // Read and clear the asserted interrupts
    volatile uint32_t uartIntStatus;
    uartIntStatus = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, uartIntStatus);

    while (UARTCharsAvail(UART0_BASE))
    {
        uint32_t c = UARTCharGet(UART0_BASE);
        for (uint32_t i = 0; i < guiCoilCount; i++)
        {
            guiCoils[i].midi.processNewDataByte(c);
        }
    }
}

void GUI::init(System* sys, void (*midiISR)(void))
{
    guiSys = sys;
    guiNxt.init(guiSys, 3, 115200, guiNxtTimeoutUS);
    for (uint32_t i = 0; i < guiCoilCount; i++)
    {
        // As of now all coils share the same init settings. However this is
        // not mandatory.
        guiCoils[i].midi.init(guiSys, 0, 115200, midiISR);
        guiCoils[i].out.init(guiSys, i);
        guiCoils[i].filteredFrequency.init(guiSys, 1.8f, 5.0f);
        guiCoils[i].filteredOntimeUS.init(guiSys, 2.0f, 30.0f);
    }

    while (guiNxt.getVal("comOk") != 1)
    {
        guiNxt.sendCmd("rest");
        guiSys->delayUS(1000000);
        guiNxt.setVal("comOk", 1);
        guiSys->delayUS(5000);
    }
    guiNxt.setVal("comOk", 2);
    guiNxt.setVal("TC_All_Stngs.maxCoilCount", guiCoilCount);
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
    // Used to detect timeouts
    uint32_t time = 0;

    // receive and store command
    if (guiNxt.charsAvail())
    {
        guiCommand = guiNxt.getChar();
        switch (guiCommand)
        {
            case 'm':
            {
                time = guiSys->getSystemTimeUS();
                while (guiNxt.charsAvail() < 2)
                {
                    if (guiSys->getSystemTimeUS() - time > guiNxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        guiSys->error();
                    }
                }
                char modeByte0 = guiNxt.getChar();
                char modeByte1 = guiNxt.getChar();
                if (modeByte0 == 'e' && modeByte1 == 'x')
                {
                    guiMode = idle;
                }
                else if (modeByte0 == 's' && modeByte1 == 'i')
                {
                    guiMode = simple;
                }
                else if (modeByte0 == 'm' && modeByte1 == 'l')
                {
                    guiMode = midiLive;
                }
                else if (modeByte0 == 'u' && modeByte1 == 's')
                {
                    guiMode = userSelect;
                }
                else if (modeByte0 == 'n' && modeByte1 == 'u')
                {
                    guiMode = nxtFWUpdate;
                }
                break;
            }
            case 'd':
            {
                time = guiSys->getSystemTimeUS();
                uint32_t i = 0;
                while (i < 5)
                {
                    if (guiNxt.charsAvail())
                    {
                        guiCommandData[i++] = guiNxt.getChar();
                    }
                    if (guiSys->getSystemTimeUS() - time > guiNxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        guiSys->error();
                    }
                }
                break;
            }
            case 'c':
            {
                time = guiSys->getSystemTimeUS();
                uint32_t i = 0;
                while (i < 3)
                {
                    if (guiNxt.charsAvail())
                    {
                        guiCommandData[i++] = guiNxt.getChar();
                    }
                    if (guiSys->getSystemTimeUS() - time > guiNxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        guiSys->error();
                    }
                }
                break;
            }
            default:
            {
                guiCommand = 0;
            }
        }
    }

    // operation
    switch (guiMode)
    {
        case userSelect:
        {
            uint32_t maxOnUS = guiNxt.getVal("All_Variables.maxOntime");
            uint32_t maxDuty = guiNxt.getVal("All_Variables.maxDuty") / 10;
            for (uint32_t i = 0; i < guiCoilCount; i++)
            {
                guiCoils[i].maxDutyPerc = maxDuty;
                guiCoils[i].maxOntimeUS = maxOnUS;
                guiCoils[i].out.setMaxDutyPerc(maxDuty);
                guiCoils[i].out.setMaxOntimeUS(maxOnUS);
            }
            guiMode = idle;
        }

        case simple:
        {
            // Apply new data
            if (guiCommand == 'd')
            {
                for (uint32_t i = 0; i < guiCoilCount; i++)
                {
                    // check if current coil is affected by command. Skip otherwise
                    // Data format documented in separate file
                    if (guiCommandData[0] & (1 << i))
                    {
                        uint32_t ontimeUS  = (guiCommandData[2] << 8) + guiCommandData[1];
                        uint32_t frequency = (guiCommandData[4] << 8) + guiCommandData[3];
                        guiCoils[i].filteredOntimeUS.setTarget(ontimeUS);
                        guiCoils[i].filteredFrequency.setTarget(frequency);
                    }
                }
                // Data applied; clear command byte.
                guiCommand = 0;
            }
            // Apply outputs
            for (uint32_t i = 0; i < guiCoilCount; i++)
            {
                float f = guiCoils[i].filteredFrequency.getFiltered();
                float o = guiCoils[i].filteredOntimeUS.getFiltered();
                guiCoils[i].out.tone(f, o);
            }
            break;
        }
        case midiLive:
        {
            // Apply new data
            // Data format documented in separate file
            if (guiCommand == 'c')
            {
                uint32_t coil = guiCommandData[0] - 1;
                uint32_t channels = (guiCommandData[2] << 8) + guiCommandData[1];
                guiCoils[coil].midi.setChannels(channels);
                // Data applied; clear command byte.
                guiCommand = 0;
            }
            else if (guiCommand == 'd')
            {
                for (uint32_t i = 0; i < guiCoilCount; i++)
                {
                    // check if current coil is affected by command. Skip otherwise
                    // Data format documented in separate file
                    if (guiCommandData[0] & (1 << i))
                    {
                        uint32_t ontimeUS  = (guiCommandData[2] << 8) + guiCommandData[1];
                        uint32_t duty = (guiCommandData[4] << 8) + guiCommandData[3];
                        guiCoils[i].midi.setOntimeUSMax(ontimeUS);
                        guiCoils[i].midi.setDutyPercMax(duty);
                        guiCoils[i].midi.setRelAbsNote(guiCommandData[5]);
                    }
                }
                // Data applied; clear command byte.
                guiCommand = 0;
            }
            for (uint32_t i = 0; i < guiCoilCount; i++)
            {
                if (!guiCoils[i].midi.isEnabled())
                {
                    guiCoils[i].midi.enable();
                    guiCoils[i].midi.play();
                }
                guiCoils[i].midi.updateFrequencyOntime();
                float f = guiCoils[i].midi.getFrequency();
                float o = guiCoils[i].midi.getOntimeUS();
                guiCoils[i].out.tone(f, o);
            }
            break;
        }
        case nxtFWUpdate:
        {
            // Stop normal operation and pass data between Nextion UART and USB UART
            // After upload is done (1 sec timeout) the uC performs a reset.

            // Stop Nextion UARTstdio operation
            guiNxt.disableStdio();

            // Stop all MIDI UARTs
            for (uint32_t i = 0; i < guiCoilCount; i++)
            {
                guiCoils[i].midi.disable();
            }
            // UARTs are supposed to be initialized.
            uint32_t nxtUARTBase = guiNxt.getUARTBase();
            uint32_t usbUARTBase = UART0_BASE;
            uint32_t baudRate = guiNxt.getBaudRate();
            // Re-init to make sure settings are correct
            // (We do assume they are already enabled. TODO)
            UARTConfigSetExpClk(nxtUARTBase, guiSys->getClockFreq(), baudRate,
                                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
            UARTConfigSetExpClk(usbUARTBase, guiSys->getClockFreq(), baudRate,
                                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
            UARTFIFOEnable(nxtUARTBase);
            UARTFIFOEnable(usbUARTBase);

            bool uploadStart = false;
            uint32_t timeUS = guiSys->getExactSystemTimeUS();
            while (42)
            {
                if (UARTCharsAvail(nxtUARTBase))
                {
                    unsigned char c = UARTCharGet(nxtUARTBase);
                    UARTCharPutNonBlocking(usbUARTBase, c);
                }
                if (UARTCharsAvail(usbUARTBase))
                {
                    uploadStart = true;
                    timeUS = guiSys->getSystemTimeUS();
                    unsigned char c = UARTCharGet(usbUARTBase);
                    UARTCharPutNonBlocking(nxtUARTBase, c);
                }
                if (uploadStart)
                {
                    if ((guiSys->getSystemTimeUS() - timeUS) > 1000000)
                    {
                        SysCtlReset();
                    }
                }
            }
            break;
        }
        default: // includes idle
        {
            // Disable all outputs
            for (uint32_t i = 0; i < guiCoilCount; i++)
            {
                guiCoils[i].midi.disable();
                guiCoils[i].out.tone(0.0f, 0.0f);
            }
            break;
        }
    }
    return true;
}

/*
bool GUI::oldUpdate()
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
*/

void GUI::applyOutput()
{
    for (uint32_t i = 0; i < guiCoilCount; i++)
    {
        if (guiCoils[i].midi.isEnabled())
        {
            guiCoils[i].midi.updateFrequencyOntime();
            float f = guiCoils[i].midi.getFrequency();
            float o = guiCoils[i].midi.getOntimeUS();
            guiCoils[i].out.tone(f, o);
        }
        else
        {
            float f = guiCoils[i].filteredFrequency.getFiltered();
            float o = guiCoils[i].filteredOntimeUS.getFiltered();
            guiCoils[i].out.tone(f, o);
        }
    }
}
