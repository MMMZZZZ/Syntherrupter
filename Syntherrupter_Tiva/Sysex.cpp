/*
 * Sysex.cpp
 *
 *  Created on: 10.04.2021
 *      Author: Max Zuidberg
 */


#include <Sysex.h>


Nextion* Sysex::nxt;


Sysex::Sysex()
{
    // TODO Auto-generated constructor stub

}

void Sysex::init(Nextion* nextion)
{
    nxt = nextion;
}

bool Sysex::checkSysex(SysexMsg& msg)
{
    /*
     * if no target is required for a parameter it should be 0.
     */

    if (!msg.number)
    {
        return false;
    }

    /*
     * arg check (see below for parameter documentation)
     */
    bool lsbOk = false;

    // float commands have the same targets as non-float commands.
    msg.number &= ~0x2000;

    // targetLSB check
    switch(msg.number)
    {
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x40:
        case 0x41:
        case 0x44:
        case 0x45:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x100:
        case 0x260:
        case 0x261:
        case 0x262:
        case 0x263:
        case 0x264:
            if (msg.targetLSB < COIL_COUNT || msg.targetLSB == WILDCARD)
            {
                lsbOk = true;
            }
            break;

        case 0x20:
        case 0x101:
        case 0x200:
        case 0x220:
        case 0x221:
        case 0x222:
        case 0x223:
        case 0x224:
            if (msg.targetLSB == 0)
            {
                lsbOk = true;
            }
            break;

        case 0x240:
        case 0x241:
        case 0x242:
        case 0x243:
        case 0x244:
            if (msg.targetLSB < 3)
            {
                lsbOk = true;
            }
            break;

        case 0x300:
        case 0x301:
        case 0x302:
        case 0x303:
            if (msg.targetLSB < MIDIProgram::DATA_POINTS)
            {
                lsbOk = true;
            }
            break;
    }
    if (!lsbOk)
    {
        return false;
    }

    bool msbOk = false;

    // targetMSB check
    switch (msg.number)
    {
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x225:
            if (msg.targetMSB == MODE_SIMPLE
                    || msg.targetMSB == MODE_MIDI_LIVE
                    || msg.targetMSB == MODE_LIGHTSABER
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x40:
        case 0x41:
        case 0x44:
        case 0x45:
            if (msg.targetMSB == 0
                    || msg.targetMSB == MODE_SIMPLE
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x66:
            if (msg.targetMSB == 0
                    || msg.targetMSB == MODE_MIDI_LIVE
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x100:
        case 0x101:
            if (msg.targetMSB == 0
                    || msg.targetMSB == MODE_LIGHTSABER
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x200:
        case 0x220:
        case 0x221:
        case 0x222:
        case 0x223:
        case 0x224:
        case 0x242:
        case 0x243:
        case 0x244:
        case 0x260:
        case 0x261:
        case 0x262:
        case 0x263:
        case 0x264:
            if (msg.targetMSB == 0)
            {
                msbOk = true;
            }
            break;

        case 0x240:
        case 0x241:
            if (msg.targetMSB < (EEPROMSettings::STR_CHAR_COUNT / 4))
            {
                msbOk = true;
            }
            break;

        case 0x300:
        case 0x301:
        case 0x302:
        case 0x303:
            if (msg.targetMSB < MIDI::MAX_PROGRAMS)
            {
                msbOk = true;
            }
            break;
    }

    return msbOk;
}

void Sysex::processSysex()
{
    SysexMsg msg = MIDI::getSysex();

    if (!msg.newMsg)
    {
        return;
    }
    if (!checkSysex(msg))
    {
        return;
    }

    switch (msg.number)
    {
        /*
         * Structure:  0x01-0x1f: System commands
         *                 0x01: request value of X
         *                 0x02: request support for X
         *                 0x10: reply to request
         *
         *             0x20- 0x3f:  "common" mode parameters (ontime, duty, ...)
         *                          may not be supported by all modes but are
         *                          likely to be required by a new mode.
         *              0x40- 0x5f: simple mode parameters
         *              0x60- 0x7f: midi live mode parameters
         *             0x100-0x12f: lightsaber mode parameters
         *             0x200-0x3ff: settings
         * If applicable, any PN + 0x2000 is the float32 version of that
         * parameter (default: int32).
         * Unless noted otherwise...
         *   * the float version is not fractional. f.ex. if int32 version is
         *     in 1/1000, the f32 version isn't.
         *   * the float version of an abstract range is from 0.0f-1.0f. If
         *     f.ex. a parameter covers a range from 0-127, the float version
         *     would equal to that value divided by 127.
         *
         * If no target is required, the value is expected to be 0 can be
         * omitted if possible. Unless noted otherwise 127 is broadcast,
         * meaning it affects all possible targets. If no broadcast is
         * supported, it is noted by an "nb".
         */

        // parameter number: (not required target byte, if target is transmitted, value must be 0 or the value specified here.)
        // [required target], ui=unsigned int, f32=float(32bit), bfx = x bit bitfield, description
        case 0x0020: // [msb=s,ml,ls][lsb=0], enable/disable mode. 0=disable, 1=enable, other=reserved
            if (msg.value.i32 == 0)
            {
                if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
                {
                    Simple::stop();
                }
                if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
                {
                    MIDI::stop();
                }
                if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == WILDCARD)
                {
                    LightSaber::stop();
                }
                if (GUI::getAcceptsData())
                {
                    if (msg.targetMSB == WILDCARD)
                    {
                        nxt->sendCmd("Settings.activeModes.val=0");
                    }
                    else
                    {
                        msg.targetMSB--;
                        nxt->sendCmd("Settings.activeModes.val&=%i", ~(1 << msg.targetMSB));
                    }
                }
            }
            else if (msg.value.i32 == 1)
            {
                if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
                {
                    Simple::start();
                }
                if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
                {
                    MIDI::start();
                }
                if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == WILDCARD)
                {
                    LightSaber::start();
                }
                if (GUI::getAcceptsData())
                {
                    if (msg.targetMSB == WILDCARD)
                    {
                        nxt->sendCmd("Settings.activeModes.val=0xff");
                    }
                    else
                    {
                        msg.targetMSB--;
                        nxt->sendCmd("Settings.activeModes.val|=%i", (1 << msg.targetMSB));
                    }
                }
            }
            break;

        case 0x0021: // [msb=s,ml,ls][lsb=coil] i32 ontime in us
            msg.value.f32 = msg.value.i32;
        case 0x2021:
            if (msg.value.f32 >= 0.0f)
            {
                uint32_t start = msg.targetLSB;
                uint32_t end = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    start = 0;
                    end = COIL_COUNT;
                }
                for (uint32_t i = start; i < end; i++)
                {
                    if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
                    {
                        Coil::allCoils[i].simple.setOntimeUS(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            nxt->sendCmd("Simple.set%i.val&=0xff00", i + 1);
                            nxt->sendCmd("Simple.set%i.val|=%i", i + 1, msg.value.f32);
                        }
                    }
                    if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
                    {
                        Coil::allCoils[i].midi.setOntimeUS(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            nxt->sendCmd("MIDI_Live.set%i.val&=0xff00", i + 1);
                            nxt->sendCmd("MIDI_Live.set%i.val|=%i", i + 1, msg.value.f32);
                        }
                    }
                    if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == WILDCARD)
                    {
                        Coil::allCoils[i].lightsaber.setOntimeUS(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            msg.value.i32 = msg.value.f32;
                            switch (i)
                            {
                                case 0:
                                    nxt->sendCmd("Lightsaber.ontimes12.val&=0x00ff");
                                    nxt->sendCmd("Lightsaber.ontimes12.val|=%i", msg.value.i32 << 16);
                                    break;
                                case 1:
                                    nxt->sendCmd("Lightsaber.ontimes12.val&=0xff00");
                                    nxt->sendCmd("Lightsaber.ontimes12.val|=%i", msg.value.i32);
                                    break;
                                case 2:
                                    nxt->sendCmd("Lightsaber.ontimes34.val&=0x00ff");
                                    nxt->sendCmd("Lightsaber.ontimes34.val|=%i", msg.value.i32 << 16);
                                    break;
                                case 3:
                                    nxt->sendCmd("Lightsaber.ontimes34.val&=0xff00");
                                    nxt->sendCmd("Lightsaber.ontimes34.val|=%i", msg.value.i32);
                                    break;
                                case 4:
                                    nxt->sendCmd("Lightsaber.ontimes56.val&=0x00ff");
                                    nxt->sendCmd("Lightsaber.ontimes56.val|=%i", msg.value.i32 << 16);
                                    break;
                                case 5:
                                    nxt->sendCmd("Lightsaber.ontimes56.val&=0xff00");
                                    nxt->sendCmd("Lightsaber.ontimes56.val|=%i", msg.value.i32);
                                    break;
                            }
                        }
                    }
                }
            }
            break;
        case 0x0022: // [msb=s,ml,ls][lsb=coil], i32 duty in 1/1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2022:
            if (msg.value.f32 >= 0.0f)
            {
                uint32_t start = msg.targetLSB;
                uint32_t end = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    start = 0;
                    end = COIL_COUNT;
                }
                for (uint32_t i = start; i < end; i++)
                {
                    if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
                    {
                        Coil::allCoils[i].simple.setDuty(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            nxt->sendCmd("Simple.set%i.val&=0xff00", i + 1);
                            uint32_t ontime = Coil::allCoils[i].simple.getOntimeUS();
                            nxt->sendCmd("Simple.set%i.val|=%i", i + 1, ontime);
                        }
                    }
                    if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
                    {
                        Coil::allCoils[i].midi.setDuty(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            nxt->sendCmd("MIDI_Live.set%i.val&=0x00ff", i + 1);
                            msg.value.ui32 = ((uint32_t) msg.value.f32) << 16;
                            nxt->sendCmd("MIDI_Live.set%i.val|=%i", i + 1, msg.value.ui32);
                        }
                    }
                }
            }
            break;
        case 0x0023: // [msb=s,ml,ls][lsb=coil], i32 BPS in Hz
            msg.value.f32 = msg.value.i32;
        case 0x2023:
            if (msg.value.f32 >= 0.0f)
            {
                if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
                {
                    uint32_t start = msg.targetLSB;
                    uint32_t end = msg.targetLSB + 1;
                    if (msg.targetLSB == WILDCARD)
                    {
                        start = 0;
                        end = COIL_COUNT;
                    }
                    for (uint32_t i = start; i < end; i++)
                    {
                        Coil::allCoils[i].simple.setFrequency(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            msg.value.ui32 = msg.value.f32;
                            nxt->sendCmd("Simple.set%i.val&=0x00ff", i + 1);
                            nxt->sendCmd("Simple.set%i.val|=%i", i + 1, msg.value.ui32 << 16);
                        }
                    }
                }
            }
            break;
        case 0x0024: // [msb=s,ml,ls][lsb=coil], i32 period in us
            msg.value.f32 = msg.value.i32;
        case 0x2024:
            if (msg.value.f32 >= 0.0f)
            {
                if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
                {
                    uint32_t start = msg.targetLSB;
                    uint32_t end = msg.targetLSB + 1;
                    if (msg.targetLSB == WILDCARD)
                    {
                        start = 0;
                        end = COIL_COUNT;
                    }
                    for (uint32_t i = start; i < end; i++)
                    {
                        msg.value.f32 = 1e6f / msg.value.f32;
                        Coil::allCoils[i].simple.setFrequency(msg.value.f32);
                        if (GUI::getAcceptsData())
                        {
                            msg.value.ui32 = msg.value.f32;
                            nxt->sendCmd("Simple.set%i.val&=0x00ff", i + 1);
                            nxt->sendCmd("Simple.set%i.val|=%i", i + 1, msg.value.ui32 << 16);
                        }
                    }
                }
            }
            break;

        case 0x0040: // (msb=s)[lsb=all coils], i32 ontime filter factor /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2040:

            break;
        case 0x0041: // (msb=s)[lsb=all coils], i32 ontime filter constant /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2041:

            break;
        case 0x0044: // (msb=s)[lsb=all coils], i32 BPS filter factor /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2044:

            break;
        case 0x0045: // (msb=s)[lsb=all coils], i32 BPS filter constant /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2045:

            break;

        case 0x0060: // (msb=ml)[lsb=coil], bf16 assigned MIDI channels
        {
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                msg.value.ui32 &= 0xffff;
                Coil::allCoils[i].midi.setChannels(msg.value.ui32);
                if (GUI::getAcceptsData())
                {
                    nxt->sendCmd("TC_Settings.coil%iChn.val&=0x0000ffff", i + 1);
                    nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, msg.value.ui32);
                }
            }
            break;
        }
        case 0x0062: // (msb=ml)[lsb=coil], ui3 pan config. 0=const, 1=lin, 2-7=reserved.
        {
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                if ((msg.value.ui32 & 0b111) == 0)
                {
                    Coil::allCoils[i].midi.setPanConstVol(true);
                    if (GUI::getAcceptsData())
                    {
                        nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, (1 << 7) << 16);
                    }
                }
                else if ((msg.value.ui32 & 0b111) == 1)
                {
                    Coil::allCoils[i].midi.setPanConstVol(false);
                    if (GUI::getAcceptsData())
                    {
                        nxt->sendCmd("TC_Settings.coil%iChn.val&=%i", i + 1, ~((1 << 7) << 16));
                    }
                }
            }
            break;
        }
        case 0x2063: // (msb=ml)[lsb=coil], i32 pan position (0-127), other = disabled.
            msg.value.i32 = msg.value.f32 * 127.0f;
        case 0x0063:
        {
            if (msg.value.i32 < 0 || msg.value.i32 > 127)
            {
                msg.value.i32 = 128;
            }
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                Coil::allCoils[i].midi.setPan(msg.value.i32);
                if (GUI::getAcceptsData())
                {
                    nxt->sendCmd("TC_Settings.coil%iChn.val&=%i", i + 1, ~(0xff << 24));
                    nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, msg.value.i32 << 24);
                }
            }
            break;
        }
        case 0x2064: // (msb=ml)[lsb=coil], i32 pan reach (0-127)
            msg.value.i32 = msg.value.f32 * 127.0f;
        case 0x0064:
        {
            if (msg.value.i32 >= 0 && msg.value.i32 <= 127)
            {
                uint32_t start = msg.targetLSB;
                uint32_t end = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    start = 0;
                    end = COIL_COUNT;
                }
                for (uint32_t i = start; i < end; i++)
                {
                    Coil::allCoils[i].midi.setPanReach(msg.value.i32);
                    if (GUI::getAcceptsData())
                    {
                        nxt->sendCmd("TC_Settings.coil%iChn.val&=%i", i + 1, ~(127 << 16));
                        nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, msg.value.ui32 << 16);
                    }
                }
            }
            break;
        }

        case 0x0066: // (msb=ml)(lsb=0), bf16 reset NRPs of given channels.
            MIDI::resetNRPs(msg.value.ui32 & 0xffff);
            break;

        case 0x0100: // (msb=ls)[lsb=coil], bf4 assigned lightsabers
        {
            msg.value.ui32 &= 0b1111;
            char* sLS = "____";
            if (GUI::getAcceptsData())
            {
                for (uint32_t i = 0; i < 4; i++)
                {
                    if (msg.value.ui32 & (1 << i))
                    {
                        sLS[i] = '1' + i;
                    }
                }
            }
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                Coil::allCoils[i].lightsaber.setActiveLightsabers(msg.value.ui32);
                if (GUI::getAcceptsData())
                {
                    nxt->setVal("Lightsabers.sLS%i.txt=%s", i + 1, sLS);
                }
            }
            break;
        }
        case 0x0101: // (msb=ls)(lsb=0), i32 assign given ID to specified lightsaber. id can be 0-3. other values are reserved.
            /*
             * Note: while the parameter is documented as i32, i32 and ui32
             * are exactly the same for legal values. Treating the value
             * as ui32 removes the necessity to check against 0 (all negative
             * i32 values will become something >> 2 billion).
             */
            if (msg.value.ui32 < 4)
            {
                LightSaber::ESPSetID(msg.value.ui32);
            }
            break;

        case 0x0200: // ()(), i32 EEPROM update mode, 0=manual, 1=force update, 2=auto (after each settings command), other=reserved.

            break;

        case 0x2220:
            msg.value.ui32 = msg.value.f32 * 100.0f;
        case 0x0220: // ()(), i32 display brightness, 0-100, other=reserved
            if (msg.value.ui32 <= 100)
            {
                nxt->setVal("dim", msg.value.ui32, Nextion::NO_EXT);
            }
            break;
        case 0x2221:
            msg.value.ui32 = msg.value.f32;
        case 0x0221: // ()(), i32 seconds til standby, 1-3600, 0=disabled, other=reserved
            if (msg.value.ui32 <= 3600)
            {
                nxt->setVal("thsp", msg.value.ui32, Nextion::NO_EXT);
            }
            break;
        case 0x2222:
            msg.value.ui32 = msg.value.f32;
        case 0x0222: // ()(), i32 button hold time (ms), 50-9999, other=reserved
            if (msg.value.ui32 >= 50 && msg.value.ui32 <= 9999)
            {
                nxt->setVal("Other_Settings.nHoldTime", msg.value.ui32);
            }
            break;
        case 0x0223: // ()(), bf1 safety options, [0]: background shutdown, 0=disabled, 1=enabled.
            nxt->setVal("Other_Settings.nBackOff", msg.value.ui32 & 0b1);
            break;
        case 0x0224: // ()(), i32 color mode, 0=light, 1=dark, other=reserved
            if (msg.value.ui32 < 2)
            {
                nxt->setVal("Settings.colorMode", msg.value.ui32);
                nxt->sendCmd("click fLoadColors,1");
            }
            break;
        case 0x0225: // [msb=s,ml,ls][lsb=0], ui apply mode. 0=manual, 1=on release, 2=immediate, other=reserved.
            if (msg.value.ui32 < 3)
            {
                if (msg.targetMSB == WILDCARD)
                {
                    nxt->setVal("Settings.applyingMode", msg.value.ui32);
                }
            }
            break;
        case 0x0240: // [msb=charGroup][lsb=user], char[4] username
            if (msg.targetLSB == 0)
            {
                // Clear entire string before filling it up with new data.
                memset(EEPROMSettings::userData[msg.targetLSB].name, 0, EEPROMSettings::STR_CHAR_COUNT);
            }
            memcpy(&(EEPROMSettings::userData[msg.targetLSB].name[msg.targetMSB * 4]), msg.value.chr, 4);
            break;
        case 0x0241: // [msb=charGroup][lsb=user], char[4] password
            if (msg.targetLSB == 0)
            {
                // Clear entire string before filling it up with new data.
                memset(EEPROMSettings::userData[msg.targetLSB].password, 0, EEPROMSettings::STR_CHAR_COUNT);
            }
            memcpy(&(EEPROMSettings::userData[msg.targetLSB].password[msg.targetMSB * 4]), msg.value.chr, 4);
            break;
        case 0x0242: // ()[lsb=user,nb], i32 user max ontime in us

            break;
        case 0x0243: // ()[lsb=user,nb], i32 user max duty in 1/1000

            break;
        case 0x0244: // ()[lsb=user,nb], i32 user max BPS in Hz

            break;
        case 0x2260:
            msg.value.i32 = msg.value.f32;
        case 0x0260: // ()[lsb=coil], i32 coil max ontime in us
        {
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                Coil::allCoils[i].setMaxOntimeUS(msg.value.ui32);
                if (GUI::getAcceptsData())
                {
                    nxt->setVal("TC_Settings.coil%iOn.val=%i", i + 1, msg.value.i32);
                }
            }
            break;
        }
        case 0x0261:
            msg.value.f32 = msg.value.i32 / 1000.0f;
        case 0x2261: // ()[lsb=coil], i32 coil max duty in 1/1000
        {
            msg.value.f32 *= 1000.0f;
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                Coil::allCoils[i].setMaxDutyPerm(msg.value.f32);
                if (GUI::getAcceptsData())
                {
                    nxt->setVal("TC_Settings.coil%iDuty.val=%i", i + 1, msg.value.f32);
                }
            }
            break;
        }
        case 0x2262:
            msg.value.i32 = msg.value.f32;
        case 0x0262: // reserved for: ()[lsb=coil], i32 coil min ontime in us
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                Coil::allCoils[i].setMinOntimeUS(msg.value.i32);
            }
            break;
        case 0x2263:
            msg.value.i32 = msg.value.f32;
        case 0x0263: // ()[lsb=coil], i32 coil min offtime in us
        {
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                Coil::allCoils[i].setMinOfftimeUS(msg.value.i32);
                if (GUI::getAcceptsData())
                {
                    uint32_t temp = EEPROMSettings::coilData[i].midiMaxVoices;
                    temp |= msg.value.ui32 << 16;
                    nxt->setVal("TC_Settings.coil%iOffVoices.val=%i", i + 1, temp);
                }
            }
            break;
        }
        case 0x0264: // ()[lsb=coil], i32 coil max MIDI voices, 1-16, ohter=reserved
            if (msg.value.ui32 < 16)
            {
                uint32_t start = msg.targetLSB;
                uint32_t end = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    start = 0;
                    end = COIL_COUNT;
                }
                for (uint32_t i = start; i < end; i++)
                {
                    Coil::allCoils[i].midi.setMaxVoices(msg.value.ui32);
                    if (GUI::getAcceptsData())
                    {
                        uint32_t temp = EEPROMSettings::coilData[i].minOfftimeUS;
                        temp |= msg.value.ui32;
                        nxt->setVal("TC_Settings.coil%iOffVoices.val=%i", i + 1, temp);
                    }
                }
            }
            break;
        case 0x0300: // (msb=program)(lsb=step), i32 envelope next step, 0-7
            if (msg.value.i32 >= 0 && msg.value.i32 < MIDIProgram::DATA_POINTS)
            {
                float& amp     = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->amplitude;
                float& dur     = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->durationUS;
                float& ntau    = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->ntau;
                MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, amp, dur, ntau, msg.value.i32);
            }
            break;
        case 0x0301: // (msb=program)(lsb=step), i32 envelope step amplitude in 1/1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2301:
            if (msg.value.f32 >= 0)
            {
                float& dur     = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->durationUS;
                float& ntau    = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->ntau;
                uint8_t& next  = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->nextStep;
                MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, msg.value.f32, dur, ntau, next);
            }
            break;
        case 0x0302: // (msb=program)(lsb=step), i32 envelope step duration in us
            msg.value.f32 = msg.value.i32;
        case 0x2302:
            if (msg.value.f32 >= 0)
            {
                float& amp     = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->amplitude;
                float& ntau    = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->ntau;
                uint8_t& next  = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->nextStep;
                MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, amp, msg.value.f32, ntau, next);
            }
            break;
        case 0x0303: // (msb=program)(lsb=step), i32 envelope step n-tau in 1/1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2303:
        {
            float& amp     = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->amplitude;
            float& dur     = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->durationUS;
            uint8_t& next  = MIDI::programs[msg.targetMSB].steps[msg.targetLSB]->nextStep;
            MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, amp, dur, msg.value.f32, next);
            break;
        }
        default:
            break;
    }
}
