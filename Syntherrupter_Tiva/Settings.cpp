/*
 * Settings.cpp
 *
 *  Created on: 10.04.2021
 *      Author: Max Zuidberg
 */


#include <Settings.h>


Settings::Settings()
{
    // TODO Auto-generated constructor stub

}

void Settings::init()
{

}

bool Settings::checkSysex(uint32_t number, uint32_t targetLSB, uint32_t targetMSB, int32_t value)
{
    /*
     * sysexNum: 1-4 groups of 7 bits
     *    0-6: parameter number LSB
     *   7-13: parameter number MSB [optional]
     *  14-20: parameter target LSB [optional, but requires parameter number MSB]
     *  21-27: parameter target MSB [optional, but requires target LSB and number MSB]
     *
     * if no target is required for a parameter it should be 0.
     */

    if (!number)
    {
        return false;
    }

    /*
     * arg check (see below for parameter documentation)
     */
    bool lsbOk = false;

    // targetLSB check
    switch(number)
    {
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x100:
        case 0x260:
        case 0x261:
        case 0x262:
        case 0x263:
        case 0x264:
            if (targetLSB >= 0 && targetLSB < COIL_COUNT)
            {
                lsbOk = true;
            }
            break;
        case 0x40:
        case 0x41:
        case 0x44:
        case 0x45:
            if (targetLSB == 127)
            {
                lsbOk = true;
            }
            break;

        case 0x26:
        case 0x27:
        case 0x65:
        case 0x101:
        case 0x200:
        case 0x220:
        case 0x221:
        case 0x222:
        case 0x223:
        case 0x224:
            if (targetLSB == 0)
            {
                lsbOk = true;
            }
            break;

        case 0x240:
        case 0x241:
        case 0x242:
        case 0x243:
        case 0x244:
            if (targetLSB >= 0 && targetLSB < 3)
            {
                lsbOk = true;
            }
            break;

        case 0x300:
        case 0x301:
        case 0x302:
        case 0x303:
            if (targetLSB >= 0 && targetLSB < MIDIProgram::DATA_POINTS)
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
    switch (number)
    {
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x26:
        case 0x27:
            if (targetMSB == MODE_SIMPLE || targetMSB == MODE_MIDI_LIVE || targetMSB == MODE_LIGHTSABER )
            {
                msbOk = true;
            }
            break;

        case 0x40:
        case 0x41:
        case 0x44:
        case 0x45:
            if (targetMSB == 0 || targetMSB == MODE_SIMPLE)
            {
                msbOk = true;
            }
            break;

        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
            if (targetMSB == 0 || targetMSB == MODE_MIDI_LIVE)
            {
                msbOk = true;
            }
            break;

        case 0x100:
        case 0x101:
            if (targetMSB == 0 || targetMSB == MODE_LIGHTSABER)
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
            if (targetMSB == 0)
            {
                msbOk = true;
            }
            break;

        case 0x240:
        case 0x241:
            if (targetMSB >= 0 && targetMSB < (EEPROMSettings::STR_CHAR_COUNT / 4))
            {
                msbOk = true;
            }
            break;

        case 0x300:
        case 0x301:
        case 0x302:
        case 0x303:
            if (targetMSB >= 0 && targetMSB < MIDI::MAX_PROGRAMS)
            {
                msbOk = true;
            }
            break;
    }

    return msbOk;
}

void Settings::processSysex()
{
    MIDI::SysexMsg msg = MIDI::getSysex();

    if (!checkSysex(msg.number, msg.targetLSB, msg.targetMSB, msg.value))
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

        // parameter number: (not required target byte, if target is transmitted, value must be 0 or the value specified here.)[required target], ui=unsigned int, f32=float(32bit), bfx = x bit bitfield, description
        case 0x20: // [msb=s,ml,ls][lsb=coil] i32 ontime in us
            if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].simple.setOntimeUS(msg.targetLSB);
            }
            if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].midi.setOntimeUS(msg.value);
            }
            if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].lightsaber.setOntimeUS(msg.targetLSB);
            }
            break;
        case 0x21: // [msb=s,ml,ls][lsb=coil], i32 duty in 1/1000
            if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].simple.setDuty(msg.targetLSB / 1e3f);
            }
            if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].midi.setDuty(msg.value);
            }
            if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == MODE_ALL)
            {

            }
            break;
        case 0x22: // [msb=s,ml,ls][lsb=coil], i32 BPS in Hz
            if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].simple.setFrequency(msg.targetLSB);
            }
            break;
        case 0x23: // [msb=s,ml,ls][lsb=coil], i32 period in us
            if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == MODE_ALL)
            {
                Coil::allCoils[msg.targetLSB].simple.setFrequency(1e6f / msg.targetLSB);
            }
            break;
        case 0x26: // [msb=s,ml,ls][lsb=0], ui apply mode. 0=manual, 1=on release, 2=immediate, other=reserved.
            if (msg.targetMSB == MODE_ALL)
            {

            }
            break;
        case 0x27: // [msb=s,ml,ls][lsb=0], enable/disable mode. 0=disable, 1=enable, other=reserved
            if (msg.value == 0)
            {
                if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == MODE_ALL)
                {
                    Simple::stop();
                }
                if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == MODE_ALL)
                {
                    MIDI::stop();
                }
                if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == MODE_ALL)
                {
                    LightSaber::stop();
                }
            }
            else if (msg.value == 1)
            {
                if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == MODE_ALL)
                {
                    Simple::start();
                }
                if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == MODE_ALL)
                {
                    MIDI::start();
                }
                if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == MODE_ALL)
                {
                    LightSaber::start();
                }
            }
            break;

        case 0x40: // (msb=s)[lsb=all coils], i32 ontime filter factor /1000

            break;
        case 0x41: // (msb=s)[lsb=all coils], i32 ontime filter constant /1000

            break;
        case 0x44: // (msb=s)[lsb=all coils], i32 BPS filter factor /1000

            break;
        case 0x45: // (msb=s)[lsb=all coils], i32 BPS filter constant /1000

            break;

        case 0x60: // (msb=ml)[lsb=coil], bf16 assigned MIDI channels
            Coil::allCoils[msg.targetLSB].midi.setChannels(msg.value & 0xffff);
            break;
        case 0x61: // (msb=ml)[lsb=coil], ui3 pan config. 0=const, 1=lin, 2-7=reserved.
            if ((msg.value & 0b111) == 0)
            {
                Coil::allCoils[msg.targetLSB].midi.setPanConstVol(true);
            }
            else if ((msg.value & 0b111) == 1)
            {
                Coil::allCoils[msg.targetLSB].midi.setPanConstVol(false);
            }
            break;
        case 0x62: // (msb=ml)[lsb=coil], i32 pan position (0-127), other = disabled.
            if (msg.value < 0 || msg.value > 127)
            {
                msg.value = 128;
            }
            Coil::allCoils[msg.targetLSB].midi.setPan(msg.value);
            break;
        case 0x63: // (msb=ml)[lsb=coil], i32 pan reach (0-127)
            if (msg.value >= 0 && msg.value >= 127)
            {
                Coil::allCoils[msg.targetLSB].midi.setPanReach(msg.value);
            }
            break;
        case 0x64: // (msb=ml)[lsb=coil], reserved for additional pan reach parameter.

            break;
        case 0x65: // (msb=ml)(lsb=0), bf16 reset NRPs of given channels.
            MIDI::resetNRPs(msg.value & 0xffff);
            break;

        case 0x100: // (msb=ls)[lsb=coil], bf4 assigned lightsabers
            Coil::allCoils[msg.targetLSB].lightsaber.setActiveLightsabers(msg.value & 0b1111);
            break;
        case 0x101: // (msb=ls)(lsb=0), i32 assign given ID to specified lightsaber. id can be 0-3. other values are reserved.
            Coil::allCoils[msg.targetLSB].lightsaber.ESPSetID(msg.value);
            break;

        case 0x200: // ()(), i32 EEPROM update mode, 0=manual, 1=force update, 2=auto (after each settings command), other=reserved.

            break;

        case 0x220: // ()(), i32 display brightness, 0-100, other=reserved

            break;
        case 0x221: // ()(), i32 seconds til standby, 1-3600, 0=disabled, other=reserved

            break;
        case 0x222: // ()(), i32 button hold time (ms), 50-9999, other=reserved

            break;
        case 0x223: // ()(), bf1 safety options, [0]: background shutdown, 0=disabled, 1=enabled.

            break;
        case 0x224: // ()(), i32 color mode, 0=light, 1=dark, other=reserved

            break;
        case 0x240: // [msb=charGroup][lsb=user], char[4] username

            break;
        case 0x241: // [msb=charGroup][lsb=user], char[4] password

            break;
        case 0x242: // ()[lsb=user,nb], i32 user max ontime in us

            break;
        case 0x243: // ()[lsb=user,nb], i32 user max duty in 1/1000

            break;
        case 0x244: // ()[lsb=user,nb], i32 user max BPS in Hz

            break;
        case 0x260: // ()[lsb=coil], i32 coil max ontime in us

            break;
        case 0x261: // ()[lsb=coil], i32 coil max duty in 1/1000

            break;
        case 0x262: // reserved for: ()[lsb=coil], i32 coil min ontime in us

            break;
        case 0x263: // ()[lsb=coil], i32 coil min offtime in us

            break;
        case 0x264: // ()[lsb=coil], i32 coil max MIDI voices, 1-16, ohter=reserved

            break;
        case 0x300: // (msb=program)(lsb=step), i32 envelope next step, 0-7
            if (msg.value >= 0 && msg.value < MIDIProgram::DATA_POINTS)
            {
                float& amp     = MIDI::programs[msg.targetMSB].amplitude[msg.targetLSB];
                float& dur     = MIDI::programs[msg.targetMSB].durationUS[msg.targetLSB];
                float& ntau    = MIDI::programs[msg.targetMSB].ntau[msg.targetLSB];
                MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, amp, dur, ntau, msg.value);
            }
            break;
        case 0x301: // (msb=program)(lsb=step), i32 envelope step amplitude in 1/1000
            if (msg.value >= 0)
            {
                float& dur     = MIDI::programs[msg.targetMSB].durationUS[msg.targetLSB];
                float& ntau    = MIDI::programs[msg.targetMSB].ntau[msg.targetLSB];
                uint32_t& next = MIDI::programs[msg.targetMSB].nextStep[msg.targetLSB];
                MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, msg.value / 1e3f, dur, ntau, next);
            }
            break;
        case 0x302: // (msb=program)(lsb=step), i32 envelope step duration in us
            if (msg.value >= 0)
            {
                float& amp     = MIDI::programs[msg.targetMSB].amplitude[msg.targetLSB];
                float& ntau    = MIDI::programs[msg.targetMSB].ntau[msg.targetLSB];
                uint32_t& next = MIDI::programs[msg.targetMSB].nextStep[msg.targetLSB];
                MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, amp, msg.value, ntau, next);
            }
            break;
        case 0x303: // (msb=program)(lsb=step), i32 envelope step n-tau in 1/1000
            float& amp     = MIDI::programs[msg.targetMSB].amplitude[msg.targetLSB];
            float& dur     = MIDI::programs[msg.targetMSB].durationUS[msg.targetLSB];
            uint32_t& next = MIDI::programs[msg.targetMSB].nextStep[msg.targetLSB];
            MIDI::programs[msg.targetMSB].setDataPoint(msg.targetLSB, amp, dur, msg.value / 1e3f, next);
            break;
        default:
            break;
    }
}
