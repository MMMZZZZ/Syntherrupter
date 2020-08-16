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
    midi.usbUartISR();
}

void GUI::midiMidiUartISR()
{
    midi.midiUartISR();
}

void GUI::init(void (*midiUsbISR)(void), void (*midiMidiISR)(void))
{
    bool cfgInEEPROM = cfg.init();
    nxt.init(3, 115200, nxtTimeoutUS);
    midi.init(0, 115200, midiUsbISR, 7, midiMidiISR);
    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        // As of now all coils share the same filter settings. However this is
        // not mandatory.
        coils[i].filteredFrequency.init(1.8f, 5.0f);
        coils[i].filteredOntimeUS.init(2.0f, 30.0f);
    }

    /* Try to modify and read a Nextion value. If this works we know the
     * Nextion is ready. If this doesn't work, something is wrong with
     * the display. Could be a bad connection or a bad firmware. Because of
     * the latter one we'll enter FW Update mode to allow to change the
     * Nextion firmware (which normally requires a working firmware to access
     * that mode).
     */
    uint32_t startTime = sys.getSystemTimeUS();
    bool nxtOK = false;
    while ((sys.getSystemTimeUS() - startTime) < nxtStartTimeoutUS && !nxtOK)
    {
        nxt.sendCmd("rest");
        sys.delayUS(700000);
        nxt.setVal("comOk", 1);
        sys.delayUS(20000);
        if (nxt.getVal("comOk") == 1)
        {
            nxtOK = true;
        }
    }

    if (!nxtOK)
    {
        mode = Mode::nxtFWUpdate;
    }
    else
    {
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
                uint32_t maxOntimeUS = cfg.getCoilsMaxOntimeUS(coil);
                uint32_t minOffUS    = cfg.getCoilsMinOffUS(coil);
                uint32_t maxVoices   = cfg.getCoilsMaxVoices(coil);
                uint32_t maxDutyPerm = cfg.getCoilsMaxDutyPerm(coil);

                if (maxOntimeUS > allCoilsMaxOntimeUS)
                {
                    allCoilsMaxOntimeUS = maxOntimeUS;
                }
                if (maxDutyPerm > allCoilsMaxDutyPerm)
                {
                    allCoilsMaxDutyPerm = maxDutyPerm;
                }

                // Apply to coil objects
                midi.setTotalMaxDutyPerm(coil, maxDutyPerm);
                midi.setMaxVoices(coil, maxVoices);
                coils[coil].out.setMaxDutyPerm(maxDutyPerm);
                coils[coil].out.setMaxOntimeUS(maxOntimeUS);
                coils[coil].one.setMaxOntimeUS(maxOntimeUS);
                coils[coil].minOffUS = minOffUS;

                // Send to Nextion
                nxt.printf("%s.coil%iOn.val=%i\xff\xff\xff",
                              AllCoilSettings, coil + 1, maxOntimeUS);
                nxt.printf("%s.coil%iOffVoices.val=%i\xff\xff\xff",
                              AllCoilSettings, coil + 1, (maxVoices << 16) + minOffUS);
                nxt.printf("%s.coil%iDuty.val=%i\xff\xff\xff",
                              AllCoilSettings, coil + 1, maxDutyPerm);
                // Give time to the UART to send the data
                sys.delayUS(20000);
            }

            // Settings of the 3 users
            const char *AllUsersPage = "User_Settings";
            for (uint32_t user = 0; user < 3; user++)
            {
                uint32_t maxOntimeUS = cfg.getUsersMaxOntimeUS(user);
                uint32_t maxBPS      = cfg.getUsersMaxBPS(user);
                uint32_t maxDutyPerm = cfg.getUsersMaxDutyPerm(user);

                if (user == 2)
                {
                    maxOntimeUS = allCoilsMaxOntimeUS;
                    maxDutyPerm = allCoilsMaxDutyPerm;
                }

                nxt.printf("%s.u%iName.txt=\"%s\"\xff\xff\xff",
                              AllUsersPage, user, cfg.userNames[user]);
                nxt.printf("%s.u%iCode.txt=\"%s\"\xff\xff\xff",
                              AllUsersPage, user, cfg.userPwds[user]);
                nxt.printf("%s.u%iOntime.val=%i\xff\xff\xff",
                              AllUsersPage, user, maxOntimeUS);
                nxt.printf("%s.u%iBPS.val=%i\xff\xff\xff",
                              AllUsersPage, user, maxBPS);
                nxt.printf("%s.u%iDuty.val=%i\xff\xff\xff",
                              AllUsersPage, user, maxDutyPerm);
                // Give time to the UART to send the data
                sys.delayUS(20000);
            }

            // Other Settings
            uint32_t buttonHoldTime =  cfg.otherSettings[0] & 0x0000ffff;
            uint32_t sleepDelay     = (cfg.otherSettings[0] & 0xffff0000) >> 16;
            uint32_t dispBrightness =  cfg.otherSettings[1] & 0xff;
            nxt.printf("Other_Settings.nHoldTime.val=%i\xff\xff\xff",
                          buttonHoldTime);
            nxt.printf("thsp=%i\xff\xff\xff", sleepDelay);
            nxt.printf("dim=%i\xff\xff\xff", dispBrightness);
        }
        nxt.setVal("TC_Settings.maxCoilCount", COIL_COUNT);
        nxt.setVal("TC_Settings.maxVoices", MAX_VOICES);

        // Give time to the UART to send the data
        sys.delayUS(20000);

        // Display Tiva firmware versions
        nxt.setTxt("tTivaFWVersion", TIVA_FW_VERSION);

        // Initialization completed.
        nxt.sendCmd("vis 255,1");

        // Show warning if no valid config is present in EEPROM. Warning is one layer below normal startup picture.
        if (!cfgInEEPROM)
        {
            nxt.sendCmd("vis pStartup,0");
        }
    }
}

void GUI::setError(const char* err)
{
    bool endOfErr = false;
    for (uint32_t i = 0; i < errorLen; i++)
    {
        if (!endOfErr)
        {
            errorTxt[i] = err[i];
            endOfErr = (err[i] == '\0');
        }
        else
        {
            errorTxt[i] = '\0';
        }
    }
}

void GUI::showError()
{
    nxt.setPage("Error");
    nxt.setTxt("tInfo", errorTxt);
}

bool GUI::checkValue(uint32_t val)
{
    if (val == nxt.receiveErrorVal)
    {
        setError("Wert unplausibel");
        return false;
    }
    if (val == nxt.receiveTimeoutVal)
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
    if (nxt.charsAvail())
    {
        command = nxt.getChar();
        switch (command)
        {
            case 'm':
            {
                time = sys.getSystemTimeUS();
                while (nxt.charsAvail() < 2)
                {
                    if (sys.getSystemTimeUS() - time > nxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        sys.error();
                    }
                }
                char modeByte0 = nxt.getChar();
                char modeByte1 = nxt.getChar();
                if (modeByte0 == 'e' && modeByte1 == 'x')
                {
                    if (mode == Mode::simple)
                    {
                        mode = Mode::simpleExit;
                    }
                    else if (mode == Mode::midiLive)
                    {
                        mode = Mode::midiLiveExit;
                    }
                    else if (mode == Mode::settings)
                    {
                        mode = Mode::settingsExit;
                    }
                }
                else if (modeByte0 == 's' && modeByte1 == 'i')
                {
                    mode = Mode::simpleEnter;
                }
                else if (modeByte0 == 'm' && modeByte1 == 'l')
                {
                    mode = Mode::midiLiveEnter;
                }
                else if (modeByte0 == 'u' && modeByte1 == 's')
                {
                    mode = Mode::userSelect;
                }
                else if (modeByte0 == 'n' && modeByte1 == 'u')
                {
                    mode = Mode::nxtFWUpdate;
                }
                else if (modeByte0 == 's' && modeByte1 == 'e')
                {
                    mode = Mode::settings;
                }
                break;
            }
            case 'd':
            {
                time = sys.getSystemTimeUS();
                uint32_t i = 0;
                while (i < 5)
                {
                    if (nxt.charsAvail())
                    {
                        commandData[i++] = nxt.getChar();
                        time = sys.getSystemTimeUS();
                    }
                    if (sys.getSystemTimeUS() - time > nxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        sys.error();
                    }
                }
                break;
            }
            case 'c':
            {
                time = sys.getSystemTimeUS();
                uint32_t i = 0;
                while (i < 3)
                {
                    if (nxt.charsAvail())
                    {
                        commandData[i++] = nxt.getChar();
                        time = sys.getSystemTimeUS();
                    }
                    if (sys.getSystemTimeUS() - time > nxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        sys.error();
                    }
                }
                break;
            }
            case 'u':
            {
                // We need at least 1 additional byte.
                time = sys.getSystemTimeUS();
                while (!nxt.charsAvail())
                {
                    if (sys.getSystemTimeUS() - time > nxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        sys.error();
                    }
                }
                commandData[0] = nxt.getChar();

                // Now we can determine the amount of data that follows and wait for it.
                uint32_t cmdLength = (commandData[0] & 0b00011111) + 1;
                uint32_t i = 1;
                while (i <= cmdLength)
                {
                    if (nxt.charsAvail())
                    {
                        commandData[i++] = nxt.getChar();
                        time = sys.getSystemTimeUS();
                    }
                    if (sys.getSystemTimeUS() - time > nxtTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        sys.error();
                    }
                }
                break;
            }
            default:
            {
                command = 0;
            }
        }
    }

    // operation
    switch (mode)
    {
    case Mode::userSelect:
        userSelect();
        break;

    case Mode::simpleEnter:
        simpleEnter();
        mode = Mode::simple;
        break;
    case Mode::simple:
        simple();
        break;
    case Mode::simpleExit:
        simpleExit();
        mode = Mode::idle;
        break;

    case Mode::midiLiveEnter:
        midiLiveEnter();
        mode = Mode::midiLive;
        break;
    case Mode::midiLive:
        midiLive();
        break;
    case Mode::midiLiveExit:
        midiLiveExit();
        mode = Mode::idle;
        break;

    case Mode::settings:
        settings();
        break;
    case Mode::settingsExit:
        settingsExit();
        mode = Mode::idle;
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
    if (command == 'd')
    {
        uint32_t user = commandData[0];
        if (user < 2)
        {
            userMaxOntimeUS = cfg.getUsersMaxOntimeUS(user);
            userMaxBPS      = cfg.getUsersMaxBPS(user);
            userMaxDutyPerm = cfg.getUsersMaxDutyPerm(user);
        }
        // Data applied; clear command byte.
        command = 0;
    }
    mode = Mode::idle;
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
        coils[i].out.init(i);
    }
}

void GUI::simple()
{
    // Apply new data
    if (command == 'd')
    {
        for (uint32_t i = 0; i < COIL_COUNT; i++)
        {
            // check if current coil is affected by command. Skip otherwise
            // Data format documented in separate file
            if (commandData[0] & (1 << i))
            {
                uint32_t ontimeUS  = (commandData[2] << 8) + commandData[1];
                uint32_t frequency = (commandData[4] << 8) + commandData[3];
                coils[i].filteredOntimeUS.setTarget(ontimeUS);
                coils[i].filteredFrequency.setTarget(frequency);
            }
        }
        // Data applied; clear command byte.
        command = 0;
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
        coils[i].one.init(i);
    }
    midi.UARTEnable(0);
    midi.UARTEnable(1);
    midi.start();
}

void GUI::midiLive()
{
    // Apply new data
    // Data format documented in separate file
    if (command == 'c')
    {
        uint32_t coil = commandData[0] - 1;
        if (coil < COIL_COUNT)
        {
            // Legacy
            uint32_t channels = (commandData[2] << 8) + commandData[1];
            midi.setChannels(coil, channels);
        }
        else
        {
            // Important date!
            uint32_t dateDDMMYYYY = (commandData[2] << 16) + (commandData[1] << 8) + commandData[0];
            if (dateDDMMYYYY == 11102161)
            {
                midi.UARTDisable();
                EEE = true;
                EET = sys.getSystemTimeUS() >> 4;
                EEI = 0;
                for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                {
                    midi.setChannels(coil, 0xffff);
                }
            }
        }
        // Data applied; clear command byte.
        command = 0;
    }
    else if (command == 'd')
    {
        uint32_t mode = (commandData[0] >> 6) & 0b11;
        if (mode == 2)
        {
            bool isMIDICommand = commandData[0] & 0b10000;
            if (isMIDICommand)
            {
                // Disable both MIDI UARTs to ensure data integrity.
                midi.UARTDisable(0);
                midi.UARTDisable(1);
                midi.addData(commandData[1]);
                midi.addData(commandData[2]);
                midi.addData(commandData[3]);
                midi.UARTEnable(0);
                midi.UARTEnable(1);
            }
            else
            {
                uint8_t channel = commandData[0] & 0xf;
                midi.UARTDisable(0);
                midi.UARTDisable(1);
                midi.addData(0xB0 + channel);    // Control Change
                midi.addData(0x63);              // NRPN Coarse
                midi.addData(commandData[1]); // Value
                midi.addData(0x62);              // NRPN Fine
                midi.addData(commandData[2]); // Value
                midi.addData(0x06);              // Data Entry Coarse
                midi.addData(commandData[3]); // Value
                midi.addData(0x26);              // Data Entry Fine
                midi.addData(commandData[4]); // Value
                midi.UARTEnable(0);
                midi.UARTEnable(1);
            }
        }
        else
        {
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                // check if current coil is affected by command. Skip otherwise
                // Data format documented in separate file
                if (commandData[0] & (1 << coil))
                {
                    if (mode == 0)
                    {
                        uint32_t channels = (commandData[2] << 8) + commandData[1];
                        midi.setChannels(coil, channels);
                        midi.setPanReach(coil, commandData[3]);
                        midi.setPan(coil, commandData[4]);
                    }
                    else if (mode == 1)
                    {
                        uint32_t channels = (commandData[2] << 8) + commandData[1];
                        midi.resetNRPs(channels);
                    }
                    else if (mode == 3)
                    {
                        uint32_t ontimeUS = (commandData[2] << 8) + commandData[1];
                        uint32_t dutyPerm = (commandData[4] << 8) + commandData[3];
                        midi.setVolSettings(coil, ontimeUS, dutyPerm);
                    }
                }
            }
        }
        // Data applied; clear command byte.
        command = 0;
    }
    if (EEE)
    {
        uint32_t time = sys.getSystemTimeUS() >> 4;
        if ((time - EET) > ((uint32_t) EED[EEI][0] * 1000))
        {
            EET = time;
            midi.addData(EED[EEI][1]);
            midi.addData(EED[EEI][2]);
            midi.addData(EED[EEI][3]);
            if (++EEI >= EES)
            {
                EEE = false;
                midi.UARTEnable(0);
                midi.UARTEnable(1);
            }
        }
    }
    midi.process();
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        uint32_t timeUS = sys.getSystemTimeUS();
        if (timeUS > coils[coil].nextAllowedFireUS)
        {
            uint32_t highestOntimeUS = 0;
            for (uint32_t note = 0; note < MAX_VOICES; note++)
            {
                if (note < midi.activeNotes[coil])
                {
                    Note *currentNote = midi.orderedNotes[coil][note];
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
    midi.UARTDisable(0);
    midi.UARTDisable(1);
    midi.stop();
    EEE = false;
}

void GUI::settings()
{
    if (command == 'd')
    {
        // Coil, user or other settings changed.
        // Data format documented in separate file.
        uint32_t settings = (commandData[0] & 0b11000000) >> 6;
        uint32_t number =    commandData[0] & 0b00111111;
        uint32_t data   =   (commandData[4] << 24)
                          + (commandData[3] << 16)
                          + (commandData[2] << 8)
                          +  commandData[1];
        if (settings == 1 && (number - 1) < COIL_COUNT)
        {
            // Coil limits. Number ranges from 1-6 instead of 0-5.
            number--;
            cfg.coilSettings[number] = data;
            midi.setTotalMaxDutyPerm(number, cfg.getCoilsMaxDutyPerm(number));
            midi.setMaxVoices(number, cfg.getCoilsMaxVoices(number));
            coils[number].minOffUS = cfg.getCoilsMinOffUS(number);
            coils[number].out.setMaxDutyPerm(cfg.getCoilsMaxDutyPerm(number));
            coils[number].out.setMaxOntimeUS(cfg.getCoilsMaxOntimeUS(number));
            coils[number].one.setMaxOntimeUS(cfg.getCoilsMaxOntimeUS(number));
        }
        else if (settings == 0 && number < 3)
        {
            // User limits.
            cfg.userSettings[number] = data;
            userMaxOntimeUS = cfg.getUsersMaxOntimeUS(number);
            userMaxBPS      = cfg.getUsersMaxBPS(number);
            userMaxDutyPerm = cfg.getUsersMaxDutyPerm(number);
        }
        else if (settings == 2 && number < 10)
        {
            // Other (general) settings
            cfg.otherSettings[number] = data;
        }
        // Data applied; clear command byte.
        command = 0;
    }
    else if (command == 'u')
    {
        uint32_t length = (commandData[0] & 0b00011111) + 1;
        bool     pwd    =  commandData[0] & 0b00100000;
        uint32_t user   = (commandData[0] & 0b11000000) >> 6;
        if (pwd)
        {
            // Data contains user password
            for (uint32_t i = 0; i < length; i++)
            {
                cfg.userPwds[user][i] = commandData[i + 1];
            }
            cfg.userPwds[user][length] = '\0';
        }
        else
        {
            // Data contains user name
            for (uint32_t i = 0; i < length; i++)
            {
                cfg.userNames[user][i] = commandData[i + 1];
            }
            cfg.userNames[user][length] = '\0';
        }
        // Data applied; clear command byte.
        command = 0;
    }
}

void GUI::settingsExit()
{
    // Update EEPROM
    cfg.update();
}

void GUI::nxtFWUpdate()
{
    // Stop normal operation and pass data between Nextion UART and USB UART
    // After upload is done (1 sec timeout) the uC performs a reset.

    // Stop Nextion UARTstdio operation
    nxt.disableStdio();

    // Stop all MIDI UARTs
    midi.UARTDisable();
    // UARTs are supposed to be initialized.
    uint32_t nxtUARTBase = nxt.getUARTBase();
    uint32_t usbUARTBase = UART0_BASE;
    uint32_t baudRate = nxt.getBaudRate();
    // Re-init to make sure settings are correct
    // (We do assume they are already enabled. TODO)
    UARTConfigSetExpClk(nxtUARTBase, sys.getClockFreq(), baudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTConfigSetExpClk(usbUARTBase, sys.getClockFreq(), baudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFOEnable(nxtUARTBase);
    UARTFIFOEnable(usbUARTBase);

    bool uploadStarted               = false;
    uint32_t timeUS                  = sys.getSystemTimeUS();
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
            timeUS = sys.getSystemTimeUS();
            if (!uploadStarted)
            {
                uploadStarted = true;
                uploadBeginUS = timeUS;
            }
            unsigned char c = UARTCharGet(usbUARTBase);
            UARTCharPutNonBlocking(nxtUARTBase, c);
        }
        if (uploadStarted && (sys.getSystemTimeUS() - timeUS) > timeoutUS)
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
