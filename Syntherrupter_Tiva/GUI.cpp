/*
 * GUI.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */

#include <GUI.h>

GUI::GUI()
{

}

GUI::~GUI()
{
}

void GUI::midiUsbUartISR()
{
    guiMidi.usbUartISR();
}

void GUI::midiMidiUartISR()
{
    guiMidi.midiUartISR();
}

void GUI::init(System* sys, void (*midiUsbISR)(void), void (*midiMidiISR)(void))
{
    guiSys = sys;
    bool cfgInEEPROM = guiCfg.init(guiSys);
    guiNxt.init(guiSys, 3, 115200, guiNxtTimeoutUS);
    guiMidi.init(guiSys, 0, 115200, midiUsbISR, 7, midiMidiISR);
    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        // As of now all coils share the same filter settings. However this is
        // not mandatory.
        coils[i].filteredFrequency.init(guiSys, 1.8f, 5.0f);
        coils[i].filteredOntimeUS.init(guiSys, 2.0f, 30.0f);
    }

    // Try to modify and read a Nextion value. If this works we know the
    // Nextion is ready.
    while (guiNxt.getVal("comOk") != 1)
    {
        guiNxt.sendCmd("rest");
        guiSys->delayUS(700000);
        guiNxt.setVal("comOk", 1);
        guiSys->delayUS(20000);
    }

    /*
     * If there is a valid configuration in the EEPROM, load it and send it to
     * the Nextion GUI. Note: While the Nextion class has methods for setting
     * component values without knowing the Nextion command structure, it is
     * easier here to create the commands directly.
     * Data format is equal to the microcontroller command format and
     * documented in a separate file.
     */
    if (cfgInEEPROM)
    {
        // User 2 ontime and duty are determined by the max coil settings.
        uint32_t allCoilsMaxOntimeUS = 0;
        uint32_t allCoilsMaxDutyPerm = 0;

        // Settings of all coils
        const char *AllCoilSettings = "TC_Settings";
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            // Load settings
            uint32_t maxOntimeUS = guiCfg.getCoilsMaxOntimeUS(coil);
            uint32_t minOffUS    = guiCfg.getCoilsMinOffUS(coil);
            uint32_t maxVoices   = guiCfg.getCoilsMaxVoices(coil);
            uint32_t maxDutyPerm = guiCfg.getCoilsMaxDutyPerm(coil);

            if (maxOntimeUS > allCoilsMaxOntimeUS)
            {
                allCoilsMaxOntimeUS = maxOntimeUS;
            }
            if (maxDutyPerm > allCoilsMaxDutyPerm)
            {
                allCoilsMaxDutyPerm = maxDutyPerm;
            }

            // Apply to coil objects
            guiMidi.setTotalMaxDutyPerm(coil, maxDutyPerm);
            guiMidi.setMaxVoices(coil, maxVoices);
            coils[coil].out.setMaxDutyPerm(maxDutyPerm);
            coils[coil].out.setMaxOntimeUS(maxOntimeUS);
            coils[coil].one.setMaxOntimeUS(maxOntimeUS);
            coils[coil].minOffUS = minOffUS;

            // Send to Nextion
            guiNxt.printf("%s.coil%iOn.val=%i\xff\xff\xff",
                          AllCoilSettings, coil + 1, maxOntimeUS);
            guiNxt.printf("%s.coil%iOffVoices.val=%i\xff\xff\xff",
                          AllCoilSettings, coil + 1, (maxVoices << 16) + minOffUS);
            guiNxt.printf("%s.coil%iDuty.val=%i\xff\xff\xff",
                          AllCoilSettings, coil + 1, maxDutyPerm);
            // Give time to the UART to send the data
            guiSys->delayUS(20000);
        }

        // Settings of the 3 users
        const char *AllUsersPage = "User_Settings";
        for (uint32_t user = 0; user < 3; user++)
        {
            uint32_t maxOntimeUS = guiCfg.getUsersMaxOntimeUS(user);
            uint32_t maxBPS      = guiCfg.getUsersMaxBPS(user);
            uint32_t maxDutyPerm = guiCfg.getUsersMaxDutyPerm(user);

            if (user == 2)
            {
                maxOntimeUS = allCoilsMaxOntimeUS;
                maxDutyPerm = allCoilsMaxDutyPerm;
            }

            guiNxt.printf("%s.u%iName.txt=\"%s\"\xff\xff\xff",
                          AllUsersPage, user, guiCfg.userNames[user]);
            guiNxt.printf("%s.u%iCode.txt=\"%s\"\xff\xff\xff",
                          AllUsersPage, user, guiCfg.userPwds[user]);
            guiNxt.printf("%s.u%iOntime.val=%i\xff\xff\xff",
                          AllUsersPage, user, maxOntimeUS);
            guiNxt.printf("%s.u%iBPS.val=%i\xff\xff\xff",
                          AllUsersPage, user, maxBPS);
            guiNxt.printf("%s.u%iDuty.val=%i\xff\xff\xff",
                          AllUsersPage, user, maxDutyPerm);
            // Give time to the UART to send the data
            guiSys->delayUS(20000);
        }

        // Other Settings
        uint32_t buttonHoldTime =  guiCfg.otherSettings[0] & 0x0000ffff;
        uint32_t sleepDelay     = (guiCfg.otherSettings[0] & 0xffff0000) >> 16;
        uint32_t dispBrightness =  guiCfg.otherSettings[1] & 0xff;
        guiNxt.printf("Other_Settings.nHoldTime.val=%i\xff\xff\xff",
                      buttonHoldTime);
        guiNxt.printf("thsp=%i\xff\xff\xff", sleepDelay);
        guiNxt.printf("dim=%i\xff\xff\xff", dispBrightness);
    }
    guiNxt.setVal("TC_Settings.maxCoilCount", COIL_COUNT);
    guiNxt.setVal("TC_Settings.maxVoices", MAX_VOICES);

    // Give time to the UART to send the data
    guiSys->delayUS(20000);

    // Display Tiva firmware versions
    guiNxt.setTxt("tTivaFWVersion", TIVA_FW_VERSION);

    // Initialization completed.
    guiNxt.sendCmd("vis 255,1");

    // Show warning if no valid config is present in EEPROM. Warning is one layer below normal startup picture.
    if (!cfgInEEPROM)
    {
        guiNxt.sendCmd("vis pStartup,0");
    }
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

void GUI::update()
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
                    if (guiMode == Mode::simple)
                    {
                        guiMode = Mode::simpleExit;
                    }
                    else if (guiMode == Mode::midiLive)
                    {
                        guiMode = Mode::midiLiveExit;
                    }
                    else if (guiMode == Mode::settings)
                    {
                        guiMode = Mode::settingsExit;
                    }
                }
                else if (modeByte0 == 's' && modeByte1 == 'i')
                {
                    guiMode = Mode::simpleEnter;
                }
                else if (modeByte0 == 'm' && modeByte1 == 'l')
                {
                    guiMode = Mode::midiLiveEnter;
                }
                else if (modeByte0 == 'u' && modeByte1 == 's')
                {
                    guiMode = Mode::userSelect;
                }
                else if (modeByte0 == 'n' && modeByte1 == 'u')
                {
                    guiMode = Mode::nxtFWUpdate;
                }
                else if (modeByte0 == 's' && modeByte1 == 'e')
                {
                    guiMode = Mode::settings;
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
                        time = guiSys->getSystemTimeUS();
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
                        time = guiSys->getSystemTimeUS();
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
            case 'u':
            {
                // We need at least 1 additional byte.
                time = guiSys->getSystemTimeUS();
                while (!guiNxt.charsAvail())
                {
                    if (guiSys->getSystemTimeUS() - time > guiNxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        guiSys->error();
                    }
                }
                guiCommandData[0] = guiNxt.getChar();

                // Now we can determine the amount of data that follows and wait for it.
                uint32_t cmdLength = (guiCommandData[0] & 0b00011111) + 1;
                uint32_t i = 1;
                while (i <= cmdLength)
                {
                    if (guiNxt.charsAvail())
                    {
                        guiCommandData[i++] = guiNxt.getChar();
                        time = guiSys->getSystemTimeUS();
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
    case Mode::userSelect:
        userSelect();
        break;

    case Mode::simpleEnter:
        simpleEnter();
        guiMode = Mode::simple;
        break;
    case Mode::simple:
        simple();
        break;
    case Mode::simpleExit:
        simpleExit();
        guiMode = Mode::idle;
        break;

    case Mode::midiLiveEnter:
        midiLiveEnter();
        guiMode = Mode::midiLive;
        break;
    case Mode::midiLive:
        midiLive();
        break;
    case Mode::midiLiveExit:
        midiLiveExit();
        guiMode = Mode::idle;
        break;

    case Mode::settings:
        settings();
        break;
    case Mode::settingsExit:
        settingsExit();
        guiMode = Mode::idle;
        break;

    case Mode::nxtFWUpdate:
        nxtFWUpdate();
        break;

    default:
        idle();
        break;
    }
}

void GUI::idle()
{
    ;
}

void GUI::userSelect()
{
    if (guiCommand == 'd')
    {
        uint32_t user = guiCommandData[0];
        if (user < 2)
        {
            guiUserMaxOntimeUS = guiCfg.getUsersMaxOntimeUS(user);
            guiUserMaxBPS      = guiCfg.getUsersMaxBPS(user);
            guiUserMaxDutyPerm = guiCfg.getUsersMaxDutyPerm(user);
        }
        // Data applied; clear command byte.
        guiCommand = 0;
    }
    guiMode = Mode::idle;
}


void GUI::simpleEnter()
{
    /*
     * In simple mode we use the Output class to generate interrupter
     * signals. It is easier and more efficient than the Oneshot class,
     * but doesn't allow polyphony.
     */
    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        coils[i].out.init(guiSys, i);
    }
}

void GUI::simple()
{
    // Apply new data
    if (guiCommand == 'd')
    {
        for (uint32_t i = 0; i < COIL_COUNT; i++)
        {
            // check if current coil is affected by command. Skip otherwise
            // Data format documented in separate file
            if (guiCommandData[0] & (1 << i))
            {
                uint32_t ontimeUS  = (guiCommandData[2] << 8) + guiCommandData[1];
                uint32_t frequency = (guiCommandData[4] << 8) + guiCommandData[3];
                coils[i].filteredOntimeUS.setTarget(ontimeUS);
                coils[i].filteredFrequency.setTarget(frequency);
            }
        }
        // Data applied; clear command byte.
        guiCommand = 0;
    }
    // Apply outputs
    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        float f = coils[i].filteredFrequency.getFiltered();
        float o = coils[i].filteredOntimeUS.getFiltered();
        coils[i].out.tone(f, o);
    }
}

void GUI::simpleExit()
{
    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        coils[i].filteredOntimeUS.setTarget(0.0f);
        coils[i].out.tone(0.0f, 0.0f);
    }
}

void GUI::midiLiveEnter()
{
    /*
     * In midiLive mode we use the Oneshot class to generate polyphonic
     * interrupter signals. While easier and more efficient to use,
     * this is not possible with the Output class.
     */
    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        coils[i].one.init(guiSys, i);
    }
    guiMidi.UARTEnable(0);
    guiMidi.UARTEnable(1);
    guiMidi.start();
}

void GUI::midiLive()
{
    // Apply new data
    // Data format documented in separate file
    if (guiCommand == 'c')
    {
        uint32_t coil = guiCommandData[0] - 1;
        if (coil < COIL_COUNT)
        {
            // Legacy
            uint32_t channels = (guiCommandData[2] << 8) + guiCommandData[1];
            guiMidi.setChannels(coil, channels);
        }
        else
        {
            // Important date!
            uint32_t dateDDMMYYYY = (guiCommandData[2] << 16) + (guiCommandData[1] << 8) + guiCommandData[0];
            if (dateDDMMYYYY == 11102161)
            {
                guiMidi.UARTDisable();
                guiEEE = true;
                guiEET = guiSys->getSystemTimeUS() >> 4;
                guiEEI = 0;
                for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                {
                    guiMidi.setChannels(coil, 0xffff);
                }
            }
        }
        // Data applied; clear command byte.
        guiCommand = 0;
    }
    else if (guiCommand == 'd')
    {
        uint32_t mode = (guiCommandData[0] >> 6) & 0b11;
        if (mode == 2)
        {
            bool isMIDICommand = guiCommandData[0] & 0b10000;
            if (isMIDICommand)
            {
                // Disable both MIDI UARTs to ensure data integrity.
                guiMidi.UARTDisable(0);
                guiMidi.UARTDisable(1);
                guiMidi.addData(guiCommandData[1]);
                guiMidi.addData(guiCommandData[2]);
                guiMidi.addData(guiCommandData[3]);
                guiMidi.UARTEnable(0);
                guiMidi.UARTEnable(1);
            }
            else
            {
                uint8_t channel = guiCommandData[0] & 0xf;
                guiMidi.UARTDisable(0);
                guiMidi.UARTDisable(1);
                guiMidi.addData(0xB0 + channel);    // Control Change
                guiMidi.addData(0x63);              // NRPN Coarse
                guiMidi.addData(guiCommandData[1]); // Value
                guiMidi.addData(0x62);              // NRPN Fine
                guiMidi.addData(guiCommandData[2]); // Value
                guiMidi.addData(0x06);              // Data Entry Coarse
                guiMidi.addData(guiCommandData[3]); // Value
                guiMidi.addData(0x26);              // Data Entry Fine
                guiMidi.addData(guiCommandData[4]); // Value
                guiMidi.UARTEnable(0);
                guiMidi.UARTEnable(1);
            }
        }
        else
        {
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                // check if current coil is affected by command. Skip otherwise
                // Data format documented in separate file
                if (guiCommandData[0] & (1 << coil))
                {
                    if (mode == 0)
                    {
                        uint32_t channels = (guiCommandData[2] << 8) + guiCommandData[1];
                        guiMidi.setChannels(coil, channels);
                        guiMidi.setPanReach(coil, guiCommandData[3]);
                        guiMidi.setPan(coil, guiCommandData[4]);
                    }
                    else if (mode == 1)
                    {
                        uint32_t channels = (guiCommandData[2] << 8) + guiCommandData[1];
                        guiMidi.resetNRPs(channels);
                    }
                    else if (mode == 3)
                    {
                        uint32_t ontimeUS = (guiCommandData[2] << 8) + guiCommandData[1];
                        uint32_t dutyPerm = (guiCommandData[4] << 8) + guiCommandData[3];
                        guiMidi.setVolSettings(coil, ontimeUS, dutyPerm);
                    }
                }
            }
        }
        // Data applied; clear command byte.
        guiCommand = 0;
    }
    if (guiEEE)
    {
        uint32_t time = guiSys->getSystemTimeUS() >> 4;
        if ((time - guiEET) > ((uint32_t) guiEED[guiEEI][0] * 1000))
        {
            guiEET = time;
            guiMidi.addData(guiEED[guiEEI][1]);
            guiMidi.addData(guiEED[guiEEI][2]);
            guiMidi.addData(guiEED[guiEEI][3]);
            if (++guiEEI >= guiEES)
            {
                guiEEE = false;
                guiMidi.UARTEnable(0);
                guiMidi.UARTEnable(1);
            }
        }
    }
    guiMidi.process();
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        uint32_t timeUS = guiSys->getSystemTimeUS();
        if (timeUS > coils[coil].nextAllowedFireUS)
        {
            uint32_t highestOntimeUS = 0;
            for (uint32_t note = 0; note < MAX_VOICES; note++)
            {
                if (note < guiMidi.activeNotes[coil])
                {
                    Note *currentNote = guiMidi.orderedNotes[coil][note];
                    if (timeUS >= currentNote->nextFireUS)
                    {
                        if (timeUS < currentNote->nextFireEndUS)
                        {
                            if (currentNote->finishedOntimeUS > highestOntimeUS)
                            {
                                highestOntimeUS = currentNote->finishedOntimeUS;
                            }
                        }
                        if (currentNote->nextFireUS)
                        {
                            currentNote->nextFireUS += currentNote->periodUS;
                        }
                        else
                        {
                            currentNote->nextFireUS = timeUS;
                        }
                        currentNote->nextFireEndUS = currentNote->nextFireUS + currentNote->periodTolUS;
                    }
                }
            }
            if (highestOntimeUS)
            {
                coils[coil].one.shot(highestOntimeUS);
                coils[coil].nextAllowedFireUS = timeUS + highestOntimeUS + coils[coil].minOffUS;
            }
        }
        /*
         * Overflow detection. No ontime or min offtime is larger than 10
         * seconds.
         * Note: In theory you'd need a similar detection for the
         * currentNote->nextFireUS variables above. In practice this means that
         * the notes playing during the overflow will stop playing but the next
         * notes will be fine. Therefore it is not worth the additional CPU
         * time to check each time for an overflow. Remember, it only happens
         * less than once an hour.
         */
        else if (coils[coil].nextAllowedFireUS - timeUS > 10000000)
        {
            coils[coil].nextAllowedFireUS = 0;
        }
    }
}

void GUI::midiLiveExit()
{
    // Stop MIDI operation
    guiMidi.UARTDisable(0);
    guiMidi.UARTDisable(1);
    guiMidi.stop();
    guiEEE = false;
}

void GUI::settings()
{
    if (guiCommand == 'd')
    {
        // Coil, user or other settings changed.
        // Data format documented in separate file.
        uint32_t settings = (guiCommandData[0] & 0b11000000) >> 6;
        uint32_t number =    guiCommandData[0] & 0b00111111;
        uint32_t data   =   (guiCommandData[4] << 24)
                          + (guiCommandData[3] << 16)
                          + (guiCommandData[2] << 8)
                          +  guiCommandData[1];
        if (settings == 1 && (number - 1) < COIL_COUNT)
        {
            // Coil limits. Number ranges from 1-6 instead of 0-5.
            number--;
            guiCfg.coilSettings[number] = data;
            guiMidi.setTotalMaxDutyPerm(number, guiCfg.getCoilsMaxDutyPerm(number));
            guiMidi.setMaxVoices(number, guiCfg.getCoilsMaxVoices(number));
            coils[number].minOffUS = guiCfg.getCoilsMinOffUS(number);
            coils[number].out.setMaxDutyPerm(guiCfg.getCoilsMaxDutyPerm(number));
            coils[number].out.setMaxOntimeUS(guiCfg.getCoilsMaxOntimeUS(number));
            coils[number].one.setMaxOntimeUS(guiCfg.getCoilsMaxOntimeUS(number));
        }
        else if (settings == 0 && number < 3)
        {
            // User limits.
            guiCfg.userSettings[number] = data;
            guiUserMaxOntimeUS = guiCfg.getUsersMaxOntimeUS(number);
            guiUserMaxBPS      = guiCfg.getUsersMaxBPS(number);
            guiUserMaxDutyPerm = guiCfg.getUsersMaxDutyPerm(number);
        }
        else if (settings == 2 && number < 10)
        {
            // Other (general) settings
            guiCfg.otherSettings[number] = data;
        }
        // Data applied; clear command byte.
        guiCommand = 0;
    }
    else if (guiCommand == 'u')
    {
        uint32_t length = (guiCommandData[0] & 0b00011111) + 1;
        bool     pwd    =  guiCommandData[0] & 0b00100000;
        uint32_t user   = (guiCommandData[0] & 0b11000000) >> 6;
        if (pwd)
        {
            // Data contains user password
            for (uint32_t i = 0; i < length; i++)
            {
                guiCfg.userPwds[user][i] = guiCommandData[i + 1];
            }
            guiCfg.userPwds[user][length] = '\0';
        }
        else
        {
            // Data contains user name
            for (uint32_t i = 0; i < length; i++)
            {
                guiCfg.userNames[user][i] = guiCommandData[i + 1];
            }
            guiCfg.userNames[user][length] = '\0';
        }
        // Data applied; clear command byte.
        guiCommand = 0;
    }
}

void GUI::settingsExit()
{
    // Update EEPROM
    guiCfg.update();
}

void GUI::nxtFWUpdate()
{
    // Stop normal operation and pass data between Nextion UART and USB UART
    // After upload is done (1 sec timeout) the uC performs a reset.

    // Stop Nextion UARTstdio operation
    guiNxt.disableStdio();

    // Stop all MIDI UARTs
    guiMidi.UARTDisable();
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

    bool uploadStarted               = false;
    uint32_t timeUS                  = guiSys->getSystemTimeUS();
    uint32_t uploadBeginUS           = timeUS;
    constexpr uint32_t minDurationUS = 15000000;
    constexpr uint32_t timeoutUS     = 1000000; // must be smaller than minUploadDurationUS
    while (42)
    {
        if (UARTCharsAvail(nxtUARTBase))
        {
            unsigned char c = UARTCharGet(nxtUARTBase);
            UARTCharPutNonBlocking(usbUARTBase, c);
        }
        if (UARTCharsAvail(usbUARTBase))
        {
            timeUS = guiSys->getSystemTimeUS();
            if (!uploadStarted)
            {
                uploadStarted = true;
                uploadBeginUS = timeUS;
            }
            unsigned char c = UARTCharGet(usbUARTBase);
            UARTCharPutNonBlocking(nxtUARTBase, c);
        }
        if (uploadStarted && (guiSys->getSystemTimeUS() - timeUS) > timeoutUS)
        {
            if (timeUS - uploadBeginUS > minDurationUS)
            {
                // Upload has happened.
                SysCtlReset();
            }
            else
            {
                // Maybe just a port check.
                uploadStarted = false;
            }
        }
    }
}
