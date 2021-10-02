/*
 * Sysex.cpp
 *
 *  Created on: 10.04.2021
 *      Author: Max Zuidberg
 */


#include <Sysex.h>


Nextion* Sysex::nxt;
uint32_t Sysex::uiUpdateMode;
SysexMsg Sysex::msg;
bool     Sysex::reading = false;
bool     Sysex::readFloat = false;
bool     Sysex::readSupportOnly = false;
bool     Sysex::readSupportConfirmed = false;
Sysex::TxMsg Sysex::txMsg;


Sysex::Sysex()
{
    // Auto-generated constructor stub
}


void Sysex::init(Nextion* nextion)
{
    nxt = nextion;
    uiUpdateMode = 2;

    txMsg.data.START  = 0xf0;
    txMsg.data.END    = 0xf7;
    txMsg.data.DMID_0 = 0x00;
    txMsg.data.DMID_1 = 0x26;
    txMsg.data.DMID_2 = 0x05;
    txMsg.data.VERSION = MIDI::SYSEX_PROTOCOL_VERSION;
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
    uint32_t number = msg.number & (~0x2000);

    // targetLSB check
    switch(number)
    {
        case 0x0002:
        case 0x0003:
        case 0x0004:
            lsbOk = true;
            break;
        case 0x0021:
        case 0x0022:
        case 0x0023:
        case 0x0024:
        case 0x0040:
        case 0x0041:
        case 0x0044:
        case 0x0045:
        case 0x0060:
        case 0x0061:
        case 0x0062:
        case 0x0063:
        case 0x0064:
        case 0x0065:
        case 0x0066:
        case 0x0100:
        case 0x0260:
        case 0x0261:
        case 0x0262:
        case 0x0263:
        case 0x0264:
            if (msg.targetLSB < COIL_COUNT || msg.targetLSB == WILDCARD)
            {
                lsbOk = true;
            }
            break;

        case 0x0067:
        case 0x0068:
        case 0x0069:
        case 0x0020:
        case 0x0101:
        case 0x0200:
        case 0x0201:
        case 0x0202:
        case 0x0203:
        case 0x0220:
        case 0x0221:
        case 0x0222:
        case 0x0223:
        case 0x0224:
        case 0x0226:
        case 0x0266:
        case 0x0267:
        case 0x0268:
            if (msg.targetLSB == 0 || msg.targetLSB == WILDCARD)
            {
                lsbOk = true;
            }
            break;

        case 0x0240:
        case 0x0241:
        case 0x0242:
        case 0x0243:
        case 0x0244:
            if (msg.targetLSB < 3)
            {
                lsbOk = true;
            }
            break;

        case 0x0300:
        case 0x0301:
        case 0x0302:
        case 0x0303:
            if (msg.targetLSB < MIDIProgram::DATA_POINTS || msg.targetMSB == WILDCARD)
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
        case 0x0002:
        case 0x0003:
        case 0x0004:
            msbOk = true;
            break;
        case 0x0225:
            if (msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;
        case 0x0020:
        case 0x0021:
        case 0x0022:
        case 0x0023:
        case 0x0024:
            if (msg.targetMSB == MODE_SIMPLE
                    || msg.targetMSB == MODE_MIDI_LIVE
                    || msg.targetMSB == MODE_LIGHTSABER
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x0040:
        case 0x0041:
        case 0x0044:
        case 0x0045:
            if (msg.targetMSB == 0
                    || msg.targetMSB == MODE_SIMPLE
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x0060:
        case 0x0061:
        case 0x0062:
        case 0x0063:
        case 0x0064:
        case 0x0066:
        case 0x0067:
        case 0x0068:
        case 0x0069:
            if (msg.targetMSB == 0
                    || msg.targetMSB == MODE_MIDI_LIVE
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x0100:
        case 0x0101:
            if (msg.targetMSB == 0
                    || msg.targetMSB == MODE_LIGHTSABER
                    || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;
        case 0x0200:
        case 0x0201:
        case 0x0202:
        case 0x0203:
        case 0x0220:
        case 0x0221:
        case 0x0222:
        case 0x0223:
        case 0x0224:
        case 0x0226:
        case 0x0242:
        case 0x0243:
        case 0x0244:
        case 0x0260:
        case 0x0261:
        case 0x0262:
        case 0x0263:
        case 0x0264:
        case 0x0266:
        case 0x0267:
        case 0x0268:
            if (msg.targetMSB == 0 || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;

        case 0x0240:
        case 0x0241:
            if (msg.targetMSB < (EEPROMSettings::STR_CHAR_COUNT / 4))
            {
                msbOk = true;
            }
            break;

        case 0x0300:
        case 0x0301:
        case 0x0302:
        case 0x0303:
            if (msg.targetMSB < MIDI::MAX_PROGRAMS || msg.targetMSB == WILDCARD)
            {
                msbOk = true;
            }
            break;
    }

    return msbOk;
}

void Sysex::processSysex()
{
    if (reading)
    {
        // if read is set, the message has already been created by the previous
        // command. Otherwise any new incoming messages are processed.

        // System commands cannot be read.
        if (msg.number <= 0x1f)
        {
            // Prevent "unsupported" reply.
            readSupportConfirmed = true;

            return;
        }

        // The outgoing data is stored within txMsg - though the value is
        // temporarily stored in msg.value (because it's otherwise unused
        // during the read operation and it already has the union aaaand
        // because it cannot be serialized directly (8->7bit conversion).
        txMsg.data.deviceID = EEPROMSettings::deviceData.deviceID;

        // Mark whether we're reading a float or non-float value
        readFloat = (msg.number >= 0x2000 && msg.number < 0x4000);
    }
    else
    {
        msg = MIDI::getSysex();
    }

    if (!msg.newMsg)
    {
        return;
    }
    if (!checkSysex(msg))
    {
        return;
    }

    // Store current EEPROM update mode in case of a forced update
    // (which doesn't change the stored update mode).
    uint8_t eepromUpdateModeOld = EEPROMSettings::deviceData.eepromUpdateMode;

    // General purpose integer temp variable
    static constexpr uint32_t INT_TMP_UNUSED = -1;
    uint32_t intTmp = INT_TMP_UNUSED;

    switch (msg.number)
    {
        /*
         * Structure of all commands is documented in this git under
         * /Documentation/Wiki/Custom MIDI Commands.md#system-exclusive-messages-sysex
         *
         * Each case has a quick one-line description of the command with the
         * following formatting:
         * parameter number: (not required target byte, if target is
         * transmitted, value must be 0 or the value specified here.)
         * [required target], ui=unsigned int, f32=float(32bit),
         * bfx = x bit bitfield, description
         */

        case 0x0002: // ()(), i32 Request if parameter is supported.
        case 0x0003: // ()(), i32 Read parameter (0x01 reply)
        case 0x0004: // ()(), i32 Get parameter (command reply)
        {
            // Targets will be set below during the read call. Response type
            // is set here.

            reading = true;
            readSupportOnly = (msg.number == 0x02);
            // reply will be a 0x01 message...
            txMsg.data.number = 0x01;
            // except if the reply shall be a command
            bool replyWithCmd = (msg.number == 0x04);

            // read range. Note: End is included!
            uint32_t readRangeStart = msg.value.ui32 & 0x7f7f;
            uint32_t readRangeEnd   = (msg.value.ui32 >> 16) & 0x7f7f;
            if (!readRangeEnd)
            {
                // No read range specified, read only single command.
                readRangeEnd = readRangeStart;
            }

            msg.number = readRangeStart;
            while (msg.number <= readRangeEnd)
            {
                if (replyWithCmd)
                {
                    txMsg.data.number = msg.number;
                }
                msg.value.ui32 = 0;
                readSupportConfirmed = false;
                processSysex();
                if (readSupportOnly && !readSupportConfirmed)
                {
                    // Command is *not* supported. Hence no confirmation has
                    // been sent so we need to communicate the missing support.
                    txMsg.data.targetLSB = msg.targetLSB;
                    txMsg.data.targetMSB = msg.targetMSB;
                    msg.value.ui32 = 0;
                    // disable readSupportOnly otherwise sendSysex will confirm
                    // support (see sendSysex)
                    readSupportOnly = false;
                    sendSysex();
                    readSupportOnly = true;
                }

                // LSB and MSB of number cannot exceed 0x7f. Therefore skip all
                // values with LSB of 0x80-0xff
                // Effectively the 8th bit of the LSB is shifted into the MSB.
                msg.number++;
                if (msg.number & 0x80)
                {
                    msg.number &= ~0x80;
                    msg.number += 0x100;
                }
            }
            reading = false;
            readSupportOnly = false;
            break;
        }

        case 0x0020: // [msb=s,ml,ls][lsb=0], enable/disable mode. 0=disable, 1=enable, other=reserved
            // Command documentation explicitly requires a 1.
            msg.value.ui32 = (msg.value.ui32 == 1);
            if (msg.targetMSB == MODE_SIMPLE || msg.targetMSB == WILDCARD)
            {
                if (reading)
                {
                    msg.value.ui32 = Simple::getRunning();
                    txMsg.data.targetLSB = 0;
                    txMsg.data.targetMSB = MODE_SIMPLE;
                    sendSysex();
                }
                else
                {
                    Simple::setRunning(msg.value.ui32);
                }
            }
            if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
            {
                if (reading)
                {
                    msg.value.ui32 = MIDI::getRunning();
                    txMsg.data.targetLSB = 0;
                    txMsg.data.targetMSB = MODE_MIDI_LIVE;
                    sendSysex();
                }
                else
                {
                    MIDI::setRunning(msg.value.ui32);
                }
            }
            if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == WILDCARD)
            {
                if (reading)
                {
                    msg.value.ui32 = LightSaber::getRunning();
                    txMsg.data.targetLSB = 0;
                    txMsg.data.targetMSB = MODE_LIGHTSABER;
                    sendSysex();
                }
                else
                {
                    LightSaber::setRunning(msg.value.ui32);
                }
            }
            if (GUI::getAcceptsData() && uiUpdateMode == 2 && !reading)
            {
                if (msg.targetMSB == WILDCARD)
                {
                    nxt->sendCmd("Settings.activeModes.val=%i", 0xff * msg.value.ui32);
                }
                else
                {
                    msg.targetMSB--;
                    nxt->sendCmd("Settings.activeModes.val&=%i", (~(1 << msg.targetMSB)) * msg.value.ui32);
                }
            }
            break;

        case 0x0021: // [msb=s,ml,ls][lsb=coil] i32 ontime in us
            msg.value.f32 = msg.value.i32;
        case 0x2021:
            if (msg.value.f32 >= 0.0f || reading)
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
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = Coil::allCoils[i].simple.getOntimeUS();
                            }
                            else
                            {
                                msg.value.ui32 = Coil::allCoils[i].simple.getOntimeUS();
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_SIMPLE;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].simple.setOntimeUS(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                nxt->sendCmd("Simple.set%i.val&=0xffff0000", i + 1);
                                nxt->sendCmd("Simple.set%i.val|=%i", i + 1, msg.value.f32);
                            }
                        }
                    }
                    if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
                    {
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = Coil::allCoils[i].midi.getOntimeUS();
                            }
                            else
                            {
                                msg.value.ui32 = Coil::allCoils[i].midi.getOntimeUS();
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_MIDI_LIVE;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].midi.setOntimeUS(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                nxt->sendCmd("MIDI_Live.set%i.val&=0xffff0000", i + 1);
                                nxt->sendCmd("MIDI_Live.set%i.val|=%i", i + 1, msg.value.f32);
                            }
                        }
                    }
                    if (msg.targetMSB == MODE_LIGHTSABER || msg.targetMSB == WILDCARD)
                    {
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = Coil::allCoils[i].lightsaber.getOntimeUS();
                            }
                            else
                            {
                                msg.value.ui32 = Coil::allCoils[i].lightsaber.getOntimeUS();
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_LIGHTSABER;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].lightsaber.setOntimeUS(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                int32_t temp = msg.value.f32;
                                switch (i)
                                {
                                    case 0:
                                        nxt->sendCmd("Lightsaber.ontimes12.val&=0x0000ffff");
                                        nxt->sendCmd("Lightsaber.ontimes12.val|=%i", temp << 16);
                                        break;
                                    case 1:
                                        nxt->sendCmd("Lightsaber.ontimes12.val&=0xffff0000");
                                        nxt->sendCmd("Lightsaber.ontimes12.val|=%i", temp);
                                        break;
                                    case 2:
                                        nxt->sendCmd("Lightsaber.ontimes34.val&=0x0000ffff");
                                        nxt->sendCmd("Lightsaber.ontimes34.val|=%i", temp << 16);
                                        break;
                                    case 3:
                                        nxt->sendCmd("Lightsaber.ontimes34.val&=0xffff0000");
                                        nxt->sendCmd("Lightsaber.ontimes34.val|=%i", temp);
                                        break;
                                    case 4:
                                        nxt->sendCmd("Lightsaber.ontimes56.val&=0x0000ffff");
                                        nxt->sendCmd("Lightsaber.ontimes56.val|=%i", temp << 16);
                                        break;
                                    case 5:
                                        nxt->sendCmd("Lightsaber.ontimes56.val&=0xffff0000");
                                        nxt->sendCmd("Lightsaber.ontimes56.val|=%i", temp);
                                        break;
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 0x0022: // [msb=s,ml,ls][lsb=coil], i32 duty in 1/1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2022:
            if (msg.value.f32 >= 0.0f || reading)
            {
                uint32_t temp = ((uint32_t) msg.value.f32) << 16;
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
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = Coil::allCoils[i].simple.getDuty();
                            }
                            else
                            {
                                msg.value.ui32 = Coil::allCoils[i].simple.getDuty() * 1e3f;
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_SIMPLE;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].simple.setDuty(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                nxt->sendCmd("Simple.set%i.val&=0xffff0000", i + 1);
                                uint32_t ontime = Coil::allCoils[i].simple.getOntimeUS();
                                nxt->sendCmd("Simple.set%i.val|=%i", i + 1, ontime);
                            }
                        }
                    }
                    if (msg.targetMSB == MODE_MIDI_LIVE || msg.targetMSB == WILDCARD)
                    {
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = Coil::allCoils[i].midi.getDuty();
                            }
                            else
                            {
                                msg.value.ui32 = Coil::allCoils[i].midi.getDuty() * 1e3f;
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_MIDI_LIVE;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].midi.setDuty(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                nxt->sendCmd("MIDI_Live.set%i.val&=0x0000ffff", i + 1);
                                nxt->sendCmd("MIDI_Live.set%i.val|=%i", i + 1, temp);
                            }
                        }
                    }
                }
            }
            break;
        case 0x0023: // [msb=s,ml,ls][lsb=coil], i32 BPS in Hz
            msg.value.f32 = msg.value.i32;
        case 0x2023:
            if (msg.value.f32 >= 0.0f || reading)
            {
                uint32_t temp = msg.value.f32;
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
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = Coil::allCoils[i].simple.getFrequency();
                            }
                            else
                            {
                                msg.value.ui32 = Coil::allCoils[i].simple.getFrequency();
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_SIMPLE;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].simple.setFrequency(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                nxt->sendCmd("Simple.set%i.val&=0x0000ffff", i + 1);
                                nxt->sendCmd("Simple.set%i.val|=%i", i + 1, temp << 16);
                            }
                        }
                    }
                }
            }
            break;
        case 0x0024: // [msb=s,ml,ls][lsb=coil], i32 period in us
            msg.value.f32 = msg.value.i32;
        case 0x2024:
            if (msg.value.f32 >= 0.0f || reading)
            {
                msg.value.f32 = 1e6f / msg.value.f32;
                uint32_t temp = ((uint32_t) msg.value.f32) << 16;
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
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = 1e6f / Coil::allCoils[i].simple.getFrequency();
                            }
                            else
                            {
                                msg.value.ui32 = 1e6f / Coil::allCoils[i].simple.getFrequency();
                            }
                            txMsg.data.targetLSB = i;
                            txMsg.data.targetMSB = MODE_SIMPLE;
                            sendSysex();
                        }
                        else
                        {
                            Coil::allCoils[i].simple.setFrequency(msg.value.f32);
                            if (GUI::getAcceptsData() && uiUpdateMode == 2)
                            {
                                nxt->sendCmd("Simple.set%i.val&=0x0000ffff", i + 1);
                                nxt->sendCmd("Simple.set%i.val|=%i", i + 1, temp << 16);
                            }
                        }
                    }
                }
            }
            break;

        case 0x0040: // (msb=s)[lsb=all coils], i32 ontime filter factor /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2040:
            if (msg.value.f32 > 0.0f || reading)
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
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = EEPROMSettings::coilData[i].simpleOntimeFF;
                        }
                        else
                        {
                            msg.value.ui32 = EEPROMSettings::coilData[i].simpleOntimeFF * 1e3f;
                        }
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = MODE_SIMPLE;
                        sendSysex();
                    }
                    else
                    {
                        EEPROMSettings::coilData[i].simpleOntimeFF = msg.value.f32;
                    }
                }
            }
            break;
        case 0x0041: // (msb=s)[lsb=all coils], i32 ontime filter constant /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2041:
            if (msg.value.f32 >= 0.0f || reading)
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
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = EEPROMSettings::coilData[i].simpleOntimeFC;
                        }
                        else
                        {
                            msg.value.ui32 = EEPROMSettings::coilData[i].simpleOntimeFC * 1e3f;
                        }
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = MODE_SIMPLE;
                        sendSysex();
                    }
                    else
                    {
                        EEPROMSettings::coilData[i].simpleOntimeFC = msg.value.f32;
                    }
                }
            }
            break;
        case 0x0044: // (msb=s)[lsb=all coils], i32 BPS filter factor /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2044:
            if (msg.value.f32 > 0.0f || reading)
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
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = EEPROMSettings::coilData[i].simpleBPSFF;
                        }
                        else
                        {
                            msg.value.ui32 = EEPROMSettings::coilData[i].simpleBPSFF * 1e3f;
                        }
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = MODE_SIMPLE;
                        sendSysex();
                    }
                    else
                    {
                        EEPROMSettings::coilData[i].simpleBPSFF = msg.value.f32;
                    }
                }
            }
            break;
        case 0x0045: // (msb=s)[lsb=all coils], i32 BPS filter constant /1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2045:
            if (msg.value.f32 >= 0.0f || reading)
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
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = EEPROMSettings::coilData[i].simpleBPSFC;
                        }
                        else
                        {
                            msg.value.ui32 = EEPROMSettings::coilData[i].simpleBPSFC * 1e3f;
                        }
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = MODE_SIMPLE;
                        sendSysex();
                    }
                    else
                    {
                        EEPROMSettings::coilData[i].simpleBPSFC = msg.value.f32;
                    }
                }
            }
            break;

        case 0x0060: // (msb=ml)[lsb=coil], bf16 assigned MIDI channels
        {
            msg.value.ui32 &= 0xffff;
            uint32_t start = msg.targetLSB;
            uint32_t end = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                start = 0;
                end = COIL_COUNT;
            }
            for (uint32_t i = start; i < end; i++)
            {
                if (reading)
                {
                    msg.value.ui32 = Coil::allCoils[i].midi.getChannels();
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = MODE_MIDI_LIVE;
                    sendSysex();
                }
                else
                {
                    Coil::allCoils[i].midi.setChannels(msg.value.ui32);
                    if (GUI::getAcceptsData() && uiUpdateMode == 2)
                    {
                        nxt->sendCmd("TC_Settings.coil%iChn.val&=0x0000ffff", i + 1);
                        nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, msg.value.ui32);
                    }
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
                if (reading)
                {
                    msg.value.ui32 = Coil::allCoils[i].midi.getPanConstVol();
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = MODE_MIDI_LIVE;
                    sendSysex();
                }
                else
                {
                    if ((msg.value.ui32 & 0b111) == 0)
                    {
                        Coil::allCoils[i].midi.setPanConstVol(true);
                        if (GUI::getAcceptsData() && uiUpdateMode == 2)
                        {
                            nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, (1 << 7) << 16);
                        }
                    }
                    else if ((msg.value.ui32 & 0b111) == 1)
                    {
                        Coil::allCoils[i].midi.setPanConstVol(false);
                        if (GUI::getAcceptsData() && uiUpdateMode == 2)
                        {
                            nxt->sendCmd("TC_Settings.coil%iChn.val&=%i", i + 1, ~((1 << 7) << 16));
                        }
                    }
                }
            }
            break;
        }
        case 0x0063: // (msb=ml)[lsb=coil], i32 pan position (0-127), other = disabled.
            intTmp = msg.value.i32;
            msg.value.f32 = intTmp / 127.0f;
        case 0x2063:
        {
            if (intTmp == INT_TMP_UNUSED)
            {
                intTmp = msg.value.f32 * 127.0f;
            }
            if (msg.value.f32 < 0.0f || msg.value.f32 > 1.0f)
            {
                msg.value.f32 = -1.0f;
                intTmp = 128;
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
                if (reading)
                {
                    if (readFloat)
                    {
                        msg.value.f32 = Coil::allCoils[i].midi.getPan();
                    }
                    else
                    {
                        int32_t tmp = Coil::allCoils[i].midi.getPan() * 127.0f;
                        msg.value.ui32 = Branchless::selectByCond(128, tmp, tmp < 0);
                    }
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = MODE_MIDI_LIVE;
                    sendSysex();
                }
                else
                {
                    Coil::allCoils[i].midi.setPan(msg.value.f32);
                    if (GUI::getAcceptsData() && uiUpdateMode == 2)
                    {
                        nxt->sendCmd("TC_Settings.coil%iChn.val&=%i", i + 1, ~(0xff << 24));
                        nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, intTmp << 24);
                    }
                }
            }
            break;
        }
        case 0x0064: // (msb=ml)[lsb=coil], i32 pan reach (0-127)
            intTmp = msg.value.ui32;
            msg.value.f32 = msg.value.ui32;
        case 0x2064:
        {
            if (intTmp == INT_TMP_UNUSED)
            {
                intTmp = msg.value.f32 * 127.0f;
            }
            if (msg.value.f32 >= 0.0f && msg.value.f32 <= 1.0f)
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
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = Coil::allCoils[i].midi.getPanReach();
                        }
                        else
                        {
                            msg.value.ui32 = Coil::allCoils[i].midi.getPanReach() * 127.0f;
                        }
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = MODE_MIDI_LIVE;
                        sendSysex();
                    }
                    else
                    {
                        Coil::allCoils[i].midi.setPanReach(msg.value.f32);
                        if (GUI::getAcceptsData() && uiUpdateMode == 2)
                        {
                            nxt->sendCmd("TC_Settings.coil%iChn.val&=%i", i + 1, ~(127 << 16));
                            nxt->sendCmd("TC_Settings.coil%iChn.val|=%i", i + 1, intTmp << 16);
                        }
                    }
                }
            }
            break;
        }

        case 0x0066: // (msb=ml)(lsb=0), bf16 reset NRPs of given channels.
            MIDI::resetNRPs(msg.value.ui32 & 0xffff);
            break;

        case 0x0067: // () (), i32 modulation depth
            msg.value.f32 = msg.value.i32 / 127.0f;
        case 0x2067:
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::deviceData.midiLfoDepth;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::deviceData.midiLfoDepth * 127.0f;
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = MODE_MIDI_LIVE;
                sendSysex();
            }
            else
            {
                if (msg.value.f32 > 1.0f)
                {
                    msg.value.f32 = 1.0f;
                }
                if (msg.value.f32 >= 0.0f)
                {
                    EEPROMSettings::deviceData.midiLfoDepth = msg.value.f32;
                }
            }
            break;

        case 0x0068: // () (), i32 modulation frequency in 1/1000 Hz
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2068:
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::deviceData.midiLfoFreq;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::deviceData.midiLfoFreq * 1e3f;
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = MODE_MIDI_LIVE;
                sendSysex();
            }
            else
            {
                if (msg.value.f32 > 0.0f && msg.value.f32 <= 1e3f)
                {
                    EEPROMSettings::deviceData.midiLfoFreq = msg.value.f32;
                }
            }
            break;

        case 0x0069: // () (), i32 modulation frequency in BPM
            msg.value.f32 = msg.value.i32;
        case 0x2069:
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::deviceData.midiLfoFreq * 60.0f;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::deviceData.midiLfoFreq * 60.0f;
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = MODE_MIDI_LIVE;
                sendSysex();
            }
            else
            {
                msg.value.f32 /= 60.0f;
                if (msg.value.f32 > 0.0f && msg.value.f32 <= 1e3f)
                {
                    EEPROMSettings::deviceData.midiLfoFreq = msg.value.f32;
                }
            }
            break;

        case 0x0100: // (msb=ls)[lsb=coil], bf4 assigned lightsabers
        {
            msg.value.ui32 &= 0b1111;
            char* sLS = "____";
            if (GUI::getAcceptsData() && uiUpdateMode == 2 && !reading)
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
                if (reading)
                {
                    msg.value.ui32 = Coil::allCoils[i].lightsaber.getActiveLightsabers();
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = MODE_LIGHTSABER;
                    sendSysex();
                }
                else
                {
                    Coil::allCoils[i].lightsaber.setActiveLightsabers(msg.value.ui32);
                    if (GUI::getAcceptsData() && uiUpdateMode == 2)
                    {
                        nxt->setVal("Lightsabers.sLS%i.txt=%s", i + 1, sLS);
                    }
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

        case 0x0200: // ()(), i32 EEPROM update mode, 0=manual, 1=force update, 2=auto (after each sysex command), other=reserved.
            if (reading)
            {
                msg.value.ui32 = EEPROMSettings::deviceData.eepromUpdateMode;
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 < 3)
                {
                    EEPROMSettings::deviceData.eepromUpdateMode = msg.value.ui32;
                }
            }
            break;
        case 0x201: // ()(), i32 sysex device id
            if (reading)
            {
                msg.value.ui32 = EEPROMSettings::deviceData.deviceID;
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 <= 126)
                {
                    EEPROMSettings::deviceData.deviceID = msg.value.ui32;
                }
            }
            break;
        case 0x2202: // ()(), i32 microcontroller reset, requires specific star date in 1/1000 to execute reset.
            msg.value.ui32 = msg.value.f32 * 1e3f;
        case 0x0202:
            if (msg.value.ui32 == 41153700)
            {
                SysCtlReset();
            }
            break;
        case 0x2203: // ()(), i32 current time in us, read only.
        case 0x0203:
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = System::getSystemTimeUS();
                }
                else
                {
                    msg.value.ui32 = System::getSystemTimeUS();
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            break;

        case 0x2220:
            msg.value.ui32 = msg.value.f32 * 100.0f;
        case 0x0220: // ()(), i32 display brightness, 0-100, other=reserved
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::deviceData.uiBrightness / 100.0f;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::deviceData.uiBrightness;
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 <= 100)
                {
                    EEPROMSettings::deviceData.uiBrightness = msg.value.ui32;
                    if (nxt->available() && uiUpdateMode == 2)
                    {
                        nxt->setVal("dim", msg.value.ui32, Nextion::NO_EXT);
                    }
                }
            }
            break;
        case 0x2221:
            msg.value.ui32 = msg.value.f32;
        case 0x0221: // ()(), i32 seconds til standby, 1-3600, 0=disabled, other=reserved
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::deviceData.uiSleepDelay;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::deviceData.uiSleepDelay;
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 <= 3600)
                {
                    EEPROMSettings::deviceData.uiSleepDelay = msg.value.ui32;
                    if (nxt->available() && uiUpdateMode == 2)
                    {
                        nxt->setVal("thsp", msg.value.ui32, Nextion::NO_EXT);
                    }
                }
            }
            break;
        case 0x2222:
            msg.value.ui32 = msg.value.f32;
        case 0x0222: // ()(), i32 button hold time (ms), 50-9999, other=reserved
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::deviceData.uiButtonHoldTime;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::deviceData.uiButtonHoldTime;
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 >= 50 && msg.value.ui32 <= 9999)
                {
                    EEPROMSettings::deviceData.uiButtonHoldTime = msg.value.ui32;
                    if (nxt->available() && uiUpdateMode == 2)
                    {
                        nxt->setVal("Other_Settings.nHoldTime", msg.value.ui32);
                    }
                }
            }
            break;
        case 0x0223: // ()(), bf1 safety options, [0]: background shutdown, 0=disabled, 1=enabled.
            if (nxt->available())
            {
                if (reading)
                {
                    msg.value.ui32 = nxt->getVal("Other_Settings.nBackOff");
                    txMsg.data.targetLSB = 0;
                    txMsg.data.targetMSB = 0;
                    sendSysex();
                }
                else
                {
                    if (nxt->available() && uiUpdateMode == 2)
                    {
                        nxt->setVal("Other_Settings.nBackOff", msg.value.ui32 & 0b1);
                    }
                }
            }
            break;
        case 0x0224: // ()(), i32 color mode, 0=light, 1=dark, other=reserved
            if (reading)
            {
                msg.value.ui32 = EEPROMSettings::deviceData.uiColorMode;
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 < 2)
                {
                    EEPROMSettings::deviceData.uiColorMode = msg.value.ui32;
                    // Currently NOT supported (requires reboot)! Needs changes on
                    // the Nextion side to work without unreasonable code copying.
                    /*
                     * if (nxt->available() && uiUpdateMode == 2)
                     * {
                     *     nxt->setVal("Settings.colorMode", msg.value.ui32);
                     *     nxt->sendCmd("click fLoadColors,1");
                     * }
                     */
                }
            }
            break;
        case 0x0225: // [msb=s&ml&ls][lsb=0], ui apply mode. 0=manual, 1=on release, 2=immediate, other=reserved.
            if (nxt->available())
            {
                if (reading)
                {
                    msg.value.ui32 = nxt->getVal("Settings.applyingMode");
                    txMsg.data.targetLSB = 0;
                    txMsg.data.targetMSB = WILDCARD;
                    sendSysex();
                }
                else
                {
                    if (msg.value.ui32 < 3)
                    {
                        if (nxt->available() && uiUpdateMode == 2)
                        {
                            nxt->setVal("Settings.applyingMode", msg.value.ui32);
                        }
                    }
                }
            }
            break;
        case 0x0226: // ()(), i32 UI update mode, 0=manual, 1=force update [NS], 2=auto (after each sysex command), other=reserved.
            if (reading)
            {
                msg.value.ui32 = uiUpdateMode;
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 == 0 || msg.value.ui32 == 2)
                {
                    uiUpdateMode = msg.value.ui32;
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
            if (GUI::getAcceptsData() && uiUpdateMode == 2)
            {
                nxt->sendCmd("User_Settings.u%iName.txt=%s",
                             msg.targetLSB,
                             EEPROMSettings::userData[msg.targetLSB].name);
            }
            break;
        case 0x0241: // [msb=charGroup][lsb=user], char[4] password
            if (msg.targetLSB == 0)
            {
                // Clear entire string before filling it up with new data.
                memset(EEPROMSettings::userData[msg.targetLSB].password, 0, EEPROMSettings::STR_CHAR_COUNT);
            }
            memcpy(&(EEPROMSettings::userData[msg.targetLSB].password[msg.targetMSB * 4]), msg.value.chr, 4);
            if (GUI::getAcceptsData() && uiUpdateMode == 2)
            {
                nxt->sendCmd("User_Settings.u%iCode.txt=%s",
                             msg.targetLSB,
                             EEPROMSettings::userData[msg.targetLSB].name);
            }
            break;
        case 0x2242:
            msg.value.i32 = msg.value.f32;
        case 0x0242: // ()[lsb=user,nb], i32 user max ontime in us
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::userData[msg.targetLSB].maxOntimeUS;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::userData[msg.targetLSB].maxOntimeUS;
                }
                txMsg.data.targetLSB = msg.targetLSB;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                EEPROMSettings::userData[msg.targetLSB].maxOntimeUS = msg.value.i32;
                if (GUI::getAcceptsData() && uiUpdateMode == 2)
                {
                    nxt->sendCmd("User_Settings.u%iOntime.val=%i", msg.targetLSB, msg.value.i32);
                }
            }
            break;
        case 0x2243:
            msg.value.i32 = msg.value.f32 * 1e3f;
        case 0x0243: // ()[lsb=user,nb], i32 user max duty in 1/1000
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::userData[msg.targetLSB].maxDutyPerm / 1e3f;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::userData[msg.targetLSB].maxDutyPerm;
                }
                txMsg.data.targetLSB = msg.targetLSB;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                EEPROMSettings::userData[msg.targetLSB].maxDutyPerm = msg.value.i32;
                if (GUI::getAcceptsData() && uiUpdateMode == 2)
                {
                    nxt->sendCmd("User_Settings.u%iDuty.val=%i", msg.targetLSB, msg.value.i32);
                }
            }
            break;
        case 0x2244:
            msg.value.i32 = msg.value.f32;
        case 0x0244: // ()[lsb=user,nb], i32 user max BPS in Hz
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = EEPROMSettings::userData[msg.targetLSB].maxBPS;
                }
                else
                {
                    msg.value.ui32 = EEPROMSettings::userData[msg.targetLSB].maxBPS;
                }
                txMsg.data.targetLSB = msg.targetLSB;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                EEPROMSettings::userData[msg.targetLSB].maxBPS = msg.value.i32;
                if (GUI::getAcceptsData() && uiUpdateMode == 2)
                {
                    nxt->sendCmd("User_Settings.u%iBPS.val=%i", msg.targetLSB, msg.value.i32);
                }
            }
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
                if (reading)
                {
                    if (readFloat)
                    {
                        msg.value.f32 = Coil::allCoils[i].getMaxOntimeUS();
                    }
                    else
                    {
                        msg.value.ui32 = Coil::allCoils[i].getMaxOntimeUS();
                    }
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = 0;
                    sendSysex();
                }
                else
                {
                    Coil::allCoils[i].setMaxOntimeUS(msg.value.ui32);
                    if (GUI::getAcceptsData() && uiUpdateMode == 2)
                    {
                        nxt->sendCmd("TC_Settings.coil%iOn.val=%i", i + 1, msg.value.i32);
                    }
                }
            }
            break;
        }
        case 0x0261:
            msg.value.f32 = msg.value.i32 / 1e3f;
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
                if (reading)
                {
                    if (readFloat)
                    {
                        msg.value.f32 = Coil::allCoils[i].getMaxDutyPerm() / 1e3f;
                    }
                    else
                    {
                        msg.value.ui32 = Coil::allCoils[i].getMaxDutyPerm();
                    }
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = 0;
                    sendSysex();
                }
                else
                {
                    Coil::allCoils[i].setMaxDutyPerm(msg.value.f32);
                    if (GUI::getAcceptsData() && uiUpdateMode == 2)
                    {
                        nxt->sendCmd("TC_Settings.coil%iDuty.val=%i", i + 1, msg.value.f32);
                    }
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
                if (reading)
                {
                    if (readFloat)
                    {
                        msg.value.f32 = Coil::allCoils[i].getMinOntimeUS();
                    }
                    else
                    {
                        msg.value.ui32 = Coil::allCoils[i].getMinOntimeUS();
                    }
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = 0;
                    sendSysex();
                }
                else
                {
                    Coil::allCoils[i].setMinOntimeUS(msg.value.i32);
                }
            }
            break;
        case 0x2263:
            msg.value.i32 = msg.value.f32;
        case 0x0263: // ()[lsb=coil], i32 coil min offtime in us
            if (msg.value.ui32 <= 0xffff || reading)
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
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = Coil::allCoils[i].getMinOfftimeUS();
                        }
                        else
                        {
                            msg.value.ui32 = Coil::allCoils[i].getMinOfftimeUS();
                        }
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = 0;
                        sendSysex();
                    }
                    else
                    {
                        Coil::allCoils[i].setMinOfftimeUS(msg.value.i32);
                        if (GUI::getAcceptsData() && uiUpdateMode == 2)
                        {
                            nxt->sendCmd("TC_Settings.coil%iOffVoics.val&=0xffff0000", i + 1);
                            nxt->sendCmd("TC_Settings.coil%iOffVoics.val|=%i", i + 1, msg.value.ui32);
                        }
                    }
                }
            }
            break;
        case 0x0264: // ()[lsb=coil], i32 coil max MIDI voices, 1-16, ohter=reserved
            if (msg.value.ui32 <= 16 || reading)
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
                    if (reading)
                    {
                        msg.value.ui32 = Coil::allCoils[i].midi.getMaxVoices();
                        txMsg.data.targetLSB = i;
                        txMsg.data.targetMSB = 0;
                        sendSysex();
                    }
                    else
                    {
                        Coil::allCoils[i].midi.setMaxVoices(msg.value.ui32);
                        if (GUI::getAcceptsData() && uiUpdateMode == 2)
                        {
                            nxt->sendCmd("TC_Settings.coil%iOffVoics.val&=0x0000ffff", i + 1);
                            nxt->sendCmd("TC_Settings.coil%iOffVoics.val|=%i", i + 1, msg.value.ui32 << 16);
                        }
                    }
                }
            }
            break;

        case 0x2266: // () (), i32 buffer duration in us
            msg.value.ui32 = msg.value.f32;
        case 0x0266:
            if (reading)
            {
                if (readFloat)
                {
                    msg.value.f32 = Coil::getBufferDurationUS();
                }
                else
                {
                    msg.value.ui32 = Coil::getBufferDurationUS();
                }
                txMsg.data.targetLSB = 0;
                txMsg.data.targetMSB = 0;
                sendSysex();
            }
            else
            {
                if (msg.value.ui32 >= 1000 && msg.value.ui32 <= 100000)
                {
                    Coil::setBufferDurationUS(msg.value.ui32);
                }
            }
            break;
        case 0x0267: // ()[lsb=coil], active voices, read only
            if (reading)
            {
                uint32_t start = msg.targetLSB;
                uint32_t end   = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    start = 0;
                    end   = COIL_COUNT;
                }
                for (uint32_t i = start; i < end; i++)
                {
                    msg.value.ui32 = Coil::allCoils[i].toneList.getActiveTones();
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = 0;
                    sendSysex();
                }
            }
            break;
        case 0x2268:
        case 0x0268: // ()[lsb=coil], output signal duty cycle, read only
            if (reading)
            {
                uint32_t start = msg.targetLSB;
                uint32_t end   = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    start = 0;
                    end   = COIL_COUNT;
                }
                for (uint32_t i = start; i < end; i++)
                {
                    if (readFloat)
                    {
                        msg.value.f32 = Coil::allCoils[i].toneList.getSignalDuty();
                    }
                    else
                    {
                        msg.value.ui32 = Coil::allCoils[i].toneList.getSignalDutyPerm();
                    }
                    txMsg.data.targetLSB = i;
                    txMsg.data.targetMSB = 0;
                    sendSysex();
                }
            }
            break;

        case 0x0300: // (msb=program)(lsb=step), i32 envelope next step, 0-7
        {
            if (msg.value.ui32 < MIDIProgram::DATA_POINTS || reading)
            {
                uint32_t progStart = msg.targetMSB;
                uint32_t progEnd   = msg.targetMSB + 1;
                if (msg.targetMSB == WILDCARD)
                {
                    progStart = 0;
                    progEnd = MIDI::MAX_PROGRAMS;
                }
                uint32_t stepStart = msg.targetLSB;
                uint32_t stepEnd   = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    stepStart = 0;
                    stepEnd = MIDIProgram::DATA_POINTS;
                }
                for (uint32_t prog = progStart; prog < progEnd; prog++)
                {
                    for (uint32_t step = stepStart; step < stepEnd; step++)
                    {
                        if (reading)
                        {
                            msg.value.ui32 = MIDI::programs[prog].steps[step]->nextStep;
                            txMsg.data.targetLSB = step;
                            txMsg.data.targetMSB = prog;
                            sendSysex();
                        }
                        else
                        {
                            float& amp     = MIDI::programs[prog].steps[step]->amplitude;
                            float& dur     = MIDI::programs[prog].steps[step]->durationUS;
                            float& ntau    = MIDI::programs[prog].steps[step]->ntau;
                            MIDI::programs[prog].setDataPoint(step, amp, dur, ntau, msg.value.i32);
                        }
                    }
                }
            }
            break;
        }
        case 0x0301: // (msb=program)(lsb=step), i32 envelope step amplitude in 1/1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2301:
        {
            if (msg.value.f32 >= 0 || reading)
            {
                uint32_t progStart = msg.targetMSB;
                uint32_t progEnd   = msg.targetMSB + 1;
                if (msg.targetMSB == WILDCARD)
                {
                    progStart = 0;
                    progEnd = MIDI::MAX_PROGRAMS;
                }
                uint32_t stepStart = msg.targetLSB;
                uint32_t stepEnd   = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    stepStart = 0;
                    stepEnd = MIDIProgram::DATA_POINTS;
                }
                for (uint32_t prog = progStart; prog < progEnd; prog++)
                {
                    for (uint32_t step = stepStart; step < stepEnd; step++)
                    {
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = MIDI::programs[prog].steps[step]->amplitude;
                            }
                            else
                            {
                                msg.value.ui32 = MIDI::programs[prog].steps[step]->amplitude * 1e3f;
                            }
                            txMsg.data.targetLSB = step;
                            txMsg.data.targetMSB = prog;
                            sendSysex();
                        }
                        else
                        {
                            float& dur     = MIDI::programs[prog].steps[step]->durationUS;
                            float& ntau    = MIDI::programs[prog].steps[step]->ntau;
                            uint8_t& next  = MIDI::programs[prog].steps[step]->nextStep;
                            MIDI::programs[prog].setDataPoint(step, msg.value.f32, dur, ntau, next);
                        }
                    }
                }
            }
            break;
        }
        case 0x0302: // (msb=program)(lsb=step), i32 envelope step duration in us
            msg.value.f32 = msg.value.i32;
        case 0x2302:
        {
            if (msg.value.f32 >= 0 || reading)
            {
                uint32_t progStart = msg.targetMSB;
                uint32_t progEnd   = msg.targetMSB + 1;
                if (msg.targetMSB == WILDCARD)
                {
                    progStart = 0;
                    progEnd = MIDI::MAX_PROGRAMS;
                }
                uint32_t stepStart = msg.targetLSB;
                uint32_t stepEnd   = msg.targetLSB + 1;
                if (msg.targetLSB == WILDCARD)
                {
                    stepStart = 0;
                    stepEnd = MIDIProgram::DATA_POINTS;
                }
                for (uint32_t prog = progStart; prog < progEnd; prog++)
                {
                    for (uint32_t step = stepStart; step < stepEnd; step++)
                    {
                        if (reading)
                        {
                            if (readFloat)
                            {
                                msg.value.f32 = MIDI::programs[prog].steps[step]->durationUS;
                            }
                            else
                            {
                                msg.value.ui32 = MIDI::programs[prog].steps[step]->durationUS;
                            }
                            txMsg.data.targetLSB = step;
                            txMsg.data.targetMSB = prog;
                            sendSysex();
                        }
                        else
                        {
                            float& amp     = MIDI::programs[prog].steps[step]->amplitude;
                            float& ntau    = MIDI::programs[prog].steps[step]->ntau;
                            uint8_t& next  = MIDI::programs[prog].steps[step]->nextStep;
                            MIDI::programs[prog].setDataPoint(step, amp, msg.value.f32, ntau, next);
                        }
                    }
                }
            }
            break;
        }
        case 0x0303: // (msb=program)(lsb=step), i32 envelope step n-tau in 1/1000
            msg.value.f32 = msg.value.i32 / 1e3f;
        case 0x2303:
        {
            uint32_t progStart = msg.targetMSB;
            uint32_t progEnd   = msg.targetMSB + 1;
            if (msg.targetMSB == WILDCARD)
            {
                progStart = 0;
                progEnd = MIDI::MAX_PROGRAMS;
            }
            uint32_t stepStart = msg.targetLSB;
            uint32_t stepEnd   = msg.targetLSB + 1;
            if (msg.targetLSB == WILDCARD)
            {
                stepStart = 0;
                stepEnd = MIDIProgram::DATA_POINTS;
            }
            for (uint32_t prog = progStart; prog < progEnd; prog++)
            {
                for (uint32_t step = stepStart; step < stepEnd; step++)
                {
                    if (reading)
                    {
                        if (readFloat)
                        {
                            msg.value.f32 = MIDI::programs[prog].steps[step]->ntau;
                        }
                        else
                        {
                            msg.value.ui32 = MIDI::programs[prog].steps[step]->ntau * 1e3f;
                        }
                        txMsg.data.targetLSB = step;
                        txMsg.data.targetMSB = prog;
                        sendSysex();
                    }
                    else
                    {
                        float& amp     = MIDI::programs[prog].steps[step]->amplitude;
                        float& dur     = MIDI::programs[prog].steps[step]->durationUS;
                        uint8_t& next  = MIDI::programs[prog].steps[step]->nextStep;
                        MIDI::programs[prog].setDataPoint(step, amp, dur, msg.value.f32, next);
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    if (EEPROMSettings::deviceData.eepromUpdateMode)
    {
        // Auto or force update

        if (EEPROMSettings::deviceData.eepromUpdateMode == 1)
        {
            // Forced update, don't change stored mode
            EEPROMSettings::deviceData.eepromUpdateMode = eepromUpdateModeOld;
        }

        EEPROMSettings::updateAll();
    }
}

void Sysex::sendSysex()
{
    /*
     * Sends the current (sysex) msg to the uart it came from.
     */

    // For a complete documentation of the encoding see wiki.


    if (readSupportOnly)
    {
        // If we entered this method we know the command
        // is supported. Hence we can confirm support.
        readSupportConfirmed = true;
        msg.value.ui32 = true;
    }
    for (uint32_t i = 0; i < 5; i++)
    {
        txMsg.data.splitValue[i] = msg.value.ui32 & 0x7f;
        msg.value.ui32 >>= 7;
    }

    // Put it into the transmit buffer (blocking)
    while (!msg.origin->write(txMsg.serialized, sizeof(txMsg.serialized)));
}
