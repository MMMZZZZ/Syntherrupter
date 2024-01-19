/*
 * GUI.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */


#include <GUI.h>


Nextion* GUI::nxt;

uint32_t GUI::state           = 0;
uint32_t GUI::command         = 0;
uint32_t GUI::commandData[33] = {0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0};

GUI::Mode GUI::mode = GUI::Mode::idle;

constexpr uint32_t GUI::errorLen;
char GUI::errorTxt[errorLen];

bool GUI::acceptsData = true;

bool GUI::EEE = false;
uint32_t GUI::EET = 0;
uint32_t GUI::EEI = 0;
constexpr uint32_t GUI::EES;
constexpr uint8_t  GUI::EED[GUI::EES][4];


GUI::GUI()
{

}

GUI::~GUI()
{

}

void GUI::init(Nextion* nextion, uint32_t cfgStatus)
{
    nxt = nextion;

    /* Try to modify and read a Nextion value. If this works we know the
     * Nextion is ready. If this doesn't work, something is wrong with
     * the display. Could be a bad connection or a bad firmware. Because of
     * the latter one we'll enter FW Update mode to allow to change the
     * Nextion firmware (which normally requires a working firmware to access
     * that mode).
     */
    uint32_t startTime = System::getSystemTimeUS();

    if (!nextion->available())
    {
        // No screen connected. Thus no need to send any data to it.
        acceptsData = false;
        return;
    }
    else
    {
        // Check if the screen got the correct firmware. Otherwise enter
        // passthrough mode to allow flashing the screen.
        nxt->setVal("comOk", 2);
        if (nxt->getVal("comOk") != 2)
        {
            mode = Mode::nxtFWUpdate;
            return;
        }

        /*
         * Send all the settings to the Nextion display.
         * Note: even if no valid config was found, meaningful
         * default values have been loaded.
         */

        // User 2 ontime and duty are determined by the max coil settings.
        uint32_t allCoilsMaxOntimeUS = 0;
        uint32_t allCoilsMaxDutyPerm = 0;

        // Settings of all coils
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            uint32_t& maxOntimeUS   = EEPROMSettings::coilData[coil].maxOntimeUS;
            uint32_t& minOfftimeUS  = EEPROMSettings::coilData[coil].minOfftimeUS;
            uint32_t& maxMidiVoices = EEPROMSettings::coilData[coil].midiMaxVoices;
            uint32_t& maxDutyPerm   = EEPROMSettings::coilData[coil].maxDutyPerm;

            if (maxOntimeUS > allCoilsMaxOntimeUS)
            {
                allCoilsMaxOntimeUS = maxOntimeUS;
            }
            if (maxDutyPerm > allCoilsMaxDutyPerm)
            {
                allCoilsMaxDutyPerm = maxDutyPerm;
            }

            // Send to Nextion
            nxt->sendCmd("TC_Settings.coil%iOn.val=%i",
                         coil + 1, maxOntimeUS);
            nxt->sendCmd("TC_Settings.coil%iOffVoics.val=%i",
                         coil + 1, (maxMidiVoices << 16) + minOfftimeUS);
            nxt->sendCmd("TC_Settings.coil%iDuty.val=%i",
                         coil + 1, maxDutyPerm);
        }

        // Settings of the 3 users
        for (uint32_t user = 0; user < 3; user++)
        {
            uint16_t& maxOntimeUS = EEPROMSettings::userData[user].maxOntimeUS;
            uint16_t& maxBPS      = EEPROMSettings::userData[user].maxBPS;
            uint16_t& maxDutyPerm = EEPROMSettings::userData[user].maxDutyPerm;

            if (user == 2)
            {
                maxOntimeUS = allCoilsMaxOntimeUS;
                maxDutyPerm = allCoilsMaxDutyPerm;
            }

            nxt->sendCmd("User_Settings.u%iName.txt=\"%s\"",
                         user, EEPROMSettings::userData[user].name);
            nxt->sendCmd("User_Settings.u%iCode.txt=\"%s\"",
                         user, EEPROMSettings::userData[user].password);
            nxt->sendCmd("User_Settings.u%iOntime.val=%i",
                         user, maxOntimeUS);
            nxt->sendCmd("User_Settings.u%iBPS.val=%i",
                         user, maxBPS);
            nxt->sendCmd("User_Settings.u%iDuty.val=%i",
                         user, maxDutyPerm);
        }

        // Other Settings
        nxt->setVal("Other_Settings.nHoldTime", EEPROMSettings::deviceData.uiButtonHoldTime);
        nxt->setVal("thsp", EEPROMSettings::deviceData.uiSleepDelay, Nextion::NO_EXT);
        nxt->setVal("dim", EEPROMSettings::deviceData.uiBrightness, Nextion::NO_EXT);
        //nxt->printf("Other_Settings.nBackOff.val=%i\xff\xff\xff", backOff);
        nxt->setVal("Settings.colorMode", EEPROMSettings::deviceData.uiColorMode);

        nxt->setVal("TC_Settings.maxCoilCount", COIL_COUNT);
        nxt->setVal("Env_Settings.maxSteps", MIDIProgram::DATA_POINTS);

        // If default values had to be loaded, inform the user.
        if (cfgStatus == EEPROMSettings::CFG_UNKNOWN)
        {
            // Show warning that a newer version has been found.
            nxt->sendCmd("tStartup.font=0");
            nxt->sendCmd("tStartup.txt=\"  Warning! EEPROM contains incompatible\\r"
                                        "  data (v%i).\\r"
                                        "  Check the online wiki for details.\"", EEPROMSettings::version);
        }
        else if (cfgStatus == EEPROMSettings::NO_CFG)
        {
            // Show warning that only default values have been loaded.
            nxt->sendCmd("tStartup.font=0");
            nxt->sendCmd("tStartup.txt=\"  Warning! EEPROM contains no data.\\r"
                                        "  Default values loaded.\\r"
                                        "  Check the online wiki for details.\"");
        }
        else if (cfgStatus == EEPROMSettings::CFG_UPGRADED)
        {
            nxt->sendCmd("tStartup.font=0");
            nxt->sendCmd("tStartup.txt=\"Warning! EEPROM format will be\\r"
                                        "upgraded. Check all values before\\r"
                                        "upgrading with Settings->Save.\"");
        }

        // Display Tiva firmware versions
        nxt->setTxt("tTivaFWVersion", TIVA_FW_VERSION);

        // Initialization completed.
        nxt->sendCmd("click comOk,1");
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
    nxt->setPage("Error");
    nxt->setTxt("tInfo", errorTxt);
}

bool GUI::checkValue(int32_t val)
{
    if (val == nxt->receiveErrorVal)
    {
        setError("Wert unplausibel");
        return false;
    }
    if (val == nxt->receiveTimeoutVal)
    {
        setError("Zeitüberschreitung");
        return false;
    }
    return true;
}

uint32_t GUI::update()
{
    /*
     * Return value:
     *   0: Emergency, outputs should be inactive
     *   1: Ok, outputs should be active.
     */

    // Used to detect timeouts
    uint32_t time = 0;

    // receive and store command
    if (nxt->charsAvail())
    {
        command = nxt->getChar();
        time = System::getSystemTimeUS();
        switch (command)
        {
            case 'm':
            {
                while (nxt->charsAvail() < 2)
                {
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
                    }
                }
                char modeByte0 = nxt->getChar();
                char modeByte1 = nxt->getChar();
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
                else if (modeByte0 == 'l' && modeByte1 == 's')
                {
                    mode = Mode::lightsaberEnter;
                }
                else if (modeByte0 == 'u' && modeByte1 == 's')
                {
                    mode = Mode::userSelect;
                }
                else if (modeByte0 == 'n' && modeByte1 == 'u')
                {
                    mode = Mode::nxtFWUpdate;
                }
                else if (modeByte0 == 'e' && modeByte1 == 'u')
                {
                    mode = Mode::espFWUpdate;
                }
                else if (modeByte0 == 's' && modeByte1 == 'e')
                {
                    mode = Mode::settings;
                }
                else if (modeByte0 == 'e' && modeByte1 == 's')
                {
                    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                    {
                        Coil::allCoils[coil].midi.setVolSettings(0.0f, 0.0f);
                        Coil::allCoils[coil].simple.setOntimeUS(0.0f, true);
                        Coil::allCoils[coil].lightsaber.setOntimeUS(0.0f);
                    }
                }
                else if (modeByte0 == 'a' && modeByte1 == 'd')
                {
                    acceptsData = true;
                }
                else if (modeByte0 == 'd' && modeByte1 == 'd')
                {
                    acceptsData = false;
                }
                else
                {
                    mode = Mode::idle;
                }
                break;
            }
            case 'd':
            {
                uint32_t i = 0;
                while (i < 5)
                {
                    if (nxt->charsAvail())
                    {
                        commandData[i++] = nxt->getChar();
                        time = System::getSystemTimeUS();
                    }
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
                    }
                }
                break;
            }
            case 'c':
            {
                uint32_t i = 0;
                while (i < 3)
                {
                    if (nxt->charsAvail())
                    {
                        commandData[i++] = nxt->getChar();
                        time = System::getSystemTimeUS();
                    }
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
                    }
                }
                break;
            }
            case 'u':
            {
                // We need at least 1 additional byte.
                while (!nxt->charsAvail())
                {
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
                    }
                }
                commandData[0] = nxt->getChar();

                // Now we can determine the amount of data that follows and wait for it.
                uint32_t cmdLength = (commandData[0] & 0b00011111) + 1;
                uint32_t i = 1;
                while (i <= cmdLength)
                {
                    if (nxt->charsAvail())
                    {
                        commandData[i++] = nxt->getChar();
                        time = System::getSystemTimeUS();
                    }
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
                    }
                }
                break;
            }
            case 0xB0 ... 0xBF:
            {
                // MIDI CC message. Notably going to be a bunch of NRP changes.
                // The particular implementation on the Nextion side transmits a total of
                // 21 bytes using running status (1 status + 20 data bytes)
                MIDI::otherBuffer.add(command);
                uint32_t i = 1;
                while (i < 21)
                {
                    if (nxt->charsAvail())
                    {
                        command = nxt->getChar();
                        MIDI::otherBuffer.add(command);
                        i++;
                    }
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
                    }
                }
                break;
            }
            case 0xF0:
            {
                // MIDI Sysex command. Hand it over to the MIDI class.
                // We'll need a couple more bytes. Usually 16 but we'll
                // only know for sure once we receive the Sysex End Marker (0xF7)
                MIDI::otherBuffer.add(command);
                while (command != 0xF7)
                {
                    if (nxt->charsAvail())
                    {
                        command = nxt->getChar();
                        MIDI::otherBuffer.add(command);
                    }
                    if (System::getSystemTimeUS() - time > Nextion::defaultTimeoutUS)
                    {
                        setError("Data timeout");
                        showError();
                        System::error();
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

    case Mode::lightsaberEnter:
        lightsaberEnter();
        mode = Mode::lightsaber;
        break;
    case Mode::lightsaber:
        lightsaber();
        break;
    case Mode::lightsaberExit:
        lightsaberExit();
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
        serialPassthrough(3);
        break;
    case Mode::espFWUpdate:
        serialPassthrough(2);
        break;

    case Mode::idle:
        break;

    default:
        mode = Mode::emergency;
        return false;
    }

    return true;
}

void GUI::userSelect()
{
    // obsolete; left for backward compatibility.
    mode = Mode::idle;
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
                Coil::allCoils[i].simple.setOntimeUS(ontimeUS);
                Coil::allCoils[i].simple.setFrequency(frequency);
            }
        }
        // Data applied; clear command byte.
        command = 0;
    }
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
            Coil::allCoils[coil].midi.setChannels(channels);
        }
        else
        {
            // Important date!
            uint32_t dateDDMMYYYY = (commandData[2] << 16) + (commandData[1] << 8) + commandData[0];
            if (dateDDMMYYYY == 11102161)
            {
                EEE = true;
                EET = System::getSystemTimeUS() >> 4;
                EEI = 0;
                for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                {
                    Coil::allCoils[coil].midi.setChannels(0xffff);
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
                MIDI::otherBuffer.add(commandData[1]);
                MIDI::otherBuffer.add(commandData[2]);
                MIDI::otherBuffer.add(commandData[3]);
            }
            else
            {
                uint8_t channel = commandData[0] & 0xf;
                MIDI::otherBuffer.add(0xB0 + channel);    // Control Change
                MIDI::otherBuffer.add(0x63);              // NRPN Coarse
                MIDI::otherBuffer.add(commandData[1]);    // Value
                MIDI::otherBuffer.add(0x62);              // NRPN Fine
                MIDI::otherBuffer.add(commandData[2]);    // Value
                MIDI::otherBuffer.add(0x06);              // Data Entry Coarse
                MIDI::otherBuffer.add(commandData[3]);    // Value
                MIDI::otherBuffer.add(0x26);              // Data Entry Fine
                MIDI::otherBuffer.add(commandData[4]);    // Value
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
                        Coil::allCoils[coil].midi.setChannels(channels);
                        if (commandData[3] & 0b10000000)
                        {
                            Coil::allCoils[coil].midi.setPanConstVol(true);
                            commandData[3] &= 0b01111111;
                        }
                        else
                        {
                            Coil::allCoils[coil].midi.setPanConstVol(false);
                        }
                        Coil::allCoils[coil].midi.setPanReach(commandData[3] / 127.0f);
                        Coil::allCoils[coil].midi.setPan(commandData[4] / 127.0f);
                    }
                    else if (mode == 1)
                    {
                        uint32_t channels = (commandData[2] << 8) + commandData[1];
                        MIDI::resetNRPs(channels);
                    }
                    else if (mode == 3)
                    {
                        uint32_t ontimeUS = (commandData[2] << 8) + commandData[1];
                        uint32_t dutyPerm = (commandData[4] << 8) + commandData[3];
                        Coil::allCoils[coil].midi.setVolSettingsPerm(ontimeUS, dutyPerm);
                    }
                }
            }
        }
        // Data applied; clear command byte.
        command = 0;
    }
    if (EEE)
    {
        uint32_t time = System::getSystemTimeUS() >> 4;
        if ((time - EET) > ((uint32_t) EED[EEI][0] * 1000))
        {
            EET = time;
            MIDI::otherBuffer.add(EED[EEI][1]);
            MIDI::otherBuffer.add(EED[EEI][2]);
            MIDI::otherBuffer.add(EED[EEI][3]);
            if (++EEI >= EES)
            {
                EEE = false;
            }
        }
    }
}

void GUI::lightsaber()
{
    if (command == 'd')
    {
        uint32_t targetCoils =  commandData[0];
        uint32_t type        =  commandData[1];
        uint32_t data1       =  commandData[2];
        uint32_t data2       = (commandData[4] << 8) + commandData[3];

        if (type == 0)
        {
            // Set which lightsabers play on this coil.
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                if (targetCoils & (1 << coil))
                {
                    Coil::allCoils[coil].lightsaber.setActiveLightsabers(data1);
                    Coil::allCoils[coil].lightsaber.setOntimeUS(data2);
                }
            }
        }
        else if (type == 1)
        {
            LightSaber::ESPSetID(data1);
        }
        // Data applied; clear command byte.
        command = 0;
    }
}

void GUI::settings()
{
    // Data format documented in separate file.
    if (command == 'd')
    {
        uint32_t settings = (commandData[0] & 0b11000000) >> 6;
        uint32_t number =    commandData[0] & 0b00111111;
        if (settings < 3)
        {
            // Coil, user or other settings changed.
            uint32_t data   =   (commandData[4] << 24)
                              + (commandData[3] << 16)
                              + (commandData[2] << 8)
                              +  commandData[1];
            if (settings == 1 && (number - 1) < COIL_COUNT)
            {
                // Coil limits. Number ranges from 1-6 instead of 0-5.
                number--;

                uint32_t maxDutyPerm   =  (data & 0xff800000) >> 23;
                uint32_t minOfftimeUS  = ((data & 0x007f0000) >> 16) * 10;
                uint32_t midiMaxVoices = ((data & 0x0000f000) >> 12) +  1;
                uint32_t maxOntimeUS   =  (data & 0x00000fff)        * 10;

                Coil::allCoils[number].setMaxDutyPerm(maxDutyPerm);
                Coil::allCoils[number].setMaxOntimeUS(maxOntimeUS);
                Coil::allCoils[number].setMinOfftimeUS(minOfftimeUS);
                Coil::allCoils[number].midi.setMaxVoices(midiMaxVoices);
            }
            else if (settings == 0 && number < 3)
            {
                // User limits.
                EEPROMSettings::userData[number].maxDutyPerm = (data & 0xff800000) >> 23;
                EEPROMSettings::userData[number].maxBPS      = ((data & 0x007ff000) >> 12) * 10;
                EEPROMSettings::userData[number].maxOntimeUS = (data & 0x00000fff) * 10;
            }
            else if (settings == 2 && number < 10)
            {
                // Other (general) settings
                if (number == 0)
                {
                    EEPROMSettings::deviceData.uiButtonHoldTime =  data        & 0xffff;
                    EEPROMSettings::deviceData.uiSleepDelay     = (data >> 16) & 0xffff;
                }
                else if (number == 1)
                {
                    EEPROMSettings::deviceData.uiBrightness =  data        & 0xff;
                    //EEPROMSettings::uiData.backOff  = (data >>  8) & 0b1;
                    EEPROMSettings::deviceData.uiColorMode  = (data >>  9) & 0b1;
                }
            }
        }
        else
        {
            // Envelope settings
            if (!number)
            {
                // Command to store current envelope settings to EEPROM
                // TODO EEPROMSettings::setMIDIPrograms();
            }
            else if (number < MIDI::MAX_PROGRAMS)
            {
                static uint32_t index{0}, nextStep{1};
                static float amplitude{1.0f}, durationUS{1000.0f}, ntau{0.0f};

                float sign = 1.0f;
                if (commandData[2] & 0b10000000)
                {
                    sign = -1.0f;
                }

                float factor = powf(10.0f, - float(commandData[2] & 0x7f));
                float val      = (commandData[4] << 8) + commandData[3];

                val *= sign * factor;

                switch (commandData[1])
                {
                    case 0: // current step
                        index      = commandData[3];
                        break;
                    case 1: // next step
                        nextStep   = commandData[3];
                        break;
                    case 2: // amplitude
                        amplitude  = val;
                        break;
                    case 3: // duration (ms)
                        durationUS = 1000.0f * val;
                        break;
                    case 4: // ntau
                        ntau       = val;
                        break;
                }

                MIDI::programs[number].setDataPoint(index, amplitude, durationUS, ntau, nextStep);
            }
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
                EEPROMSettings::userData[user].password[i] = commandData[i + 1];
            }
            EEPROMSettings::userData[user].password[length] = '\0';
        }
        else
        {
            // Data contains user name
            for (uint32_t i = 0; i < length; i++)
            {
                EEPROMSettings::userData[user].name[i] = commandData[i + 1];
            }
            EEPROMSettings::userData[user].name[length] = '\0';
        }
        // Data applied; clear command byte.
        command = 0;
    }
}

void GUI::serialPassthrough(uint32_t uartNum)
{
    /*
     * Disable Interrupts and UARTs and pass data between the USB serial port
     * and the selected serial port.
     *
     * This is done via polling to operate independantly of the baud rate
     * Since Syntherrupter does nothing else during this time, this doesn't
     * cause timing issues.
     *
     * Note: To leave this mode you have to power cycle Syntherrupter.
     */

    IntMasterDisable();

    // USB UART is UART 0
    uint32_t USBPort     = UART::UART_MAPPING[0][UART::UART_PORT_BASE];
    uint32_t USBRXPin    = UART::UART_MAPPING[0][UART::UART_RX_PIN];
    uint32_t USBTXPin    = UART::UART_MAPPING[0][UART::UART_TX_PIN];
    uint32_t targetPort  = UART::UART_MAPPING[uartNum][UART::UART_PORT_BASE];
    uint32_t targetRXPin = UART::UART_MAPPING[uartNum][UART::UART_RX_PIN];
    uint32_t targetTXPin = UART::UART_MAPPING[uartNum][UART::UART_TX_PIN];

    SysCtlPeripheralDisable(UART::UART_MAPPING[0][UART::UART_SYSCTL_PERIPH]);
    SysCtlPeripheralDisable(UART::UART_MAPPING[uartNum][UART::UART_SYSCTL_PERIPH]);

    GPIOPinTypeGPIOOutput(USBPort,    USBTXPin);
    GPIOPinTypeGPIOOutput(targetPort, targetTXPin);
    GPIOPinTypeGPIOInput(USBPort,    USBRXPin);
    GPIOPinTypeGPIOInput(targetPort, targetRXPin);
    GPIOPadConfigSet(USBPort, USBRXPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(USBPort, USBRXPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


    while (42)
    {
        uint8_t USBRXState    = 0;
        uint8_t targetRXState = 0;

        // Read Pins
        if (GPIOPinRead(USBPort, USBRXPin) & 0xff)
        {
            USBRXState    = 0xff;
        }
        else
        {
            USBRXState = 0;
        }
        if (GPIOPinRead(targetPort, targetRXPin) & 0xff)
        {
            targetRXState = 0xff;
        }
        else
        {
            targetRXState = 0;
        }

        // Pass to other port
        GPIOPinWrite(USBPort,    USBTXPin,    targetRXState);
        GPIOPinWrite(targetPort, targetTXPin, USBRXState);
    }
}
