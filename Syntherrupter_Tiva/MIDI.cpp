/*
 * MIDI.cpp
 *
 *  Created on: 27.03.2020
 *      Author: Max Zuidberg
 */

#include <MIDI.h>


MIDI::MIDI()
{
    // TODO Auto-generated constructor stub

}

MIDI::~MIDI()
{
    // TODO Auto-generated destructor stub
}

void MIDI::init(uint32_t usbUartNum, uint32_t baudRate, void (*usbISR)(void), uint32_t uartNum, void(*iSR)(void))
{
    // Enable MIDI receiving over the USB UART (selectable baud rate) and a separate MIDI UART (31250 fixed baud rate).
    uSBUARTNum = usbUartNum;
    MIDUARTNum = uartNum;
    uSBUARTBase = UART_MAPPING[uSBUARTNum][UART_BASE];
    MIDUARTBase = UART_MAPPING[MIDUARTNum][UART_BASE];
    SysCtlPeripheralEnable(UART_MAPPING[uSBUARTNum][UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(UART_MAPPING[MIDUARTNum][UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(UART_MAPPING[uSBUARTNum][UART_PORT_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(UART_MAPPING[MIDUARTNum][UART_PORT_SYSCTL_PERIPH]);
    SysCtlDelay(3);
    GPIOPinConfigure(UART_MAPPING[uSBUARTNum][UART_RX_PIN_CFG]);
    GPIOPinConfigure(UART_MAPPING[MIDUARTNum][UART_RX_PIN_CFG]);
    GPIOPinConfigure(UART_MAPPING[uSBUARTNum][UART_TX_PIN_CFG]);
    GPIOPinConfigure(UART_MAPPING[MIDUARTNum][UART_TX_PIN_CFG]);
    GPIOPinTypeUART(   UART_MAPPING[uSBUARTNum][UART_PORT_BASE],
                       UART_MAPPING[uSBUARTNum][UART_TX_PIN]
                     | UART_MAPPING[uSBUARTNum][UART_RX_PIN]);
    GPIOPinTypeUART(   UART_MAPPING[MIDUARTNum][UART_PORT_BASE],
                       UART_MAPPING[MIDUARTNum][UART_TX_PIN]
                     | UART_MAPPING[MIDUARTNum][UART_RX_PIN]);

    UARTConfigSetExpClk(uSBUARTBase, sys.getClockFreq(), baudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTConfigSetExpClk(MIDUARTBase, sys.getClockFreq(), 31250,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFODisable(uSBUARTBase);
    UARTFIFODisable(MIDUARTBase);
    UARTIntRegister(uSBUARTBase, usbISR);
    UARTIntRegister(MIDUARTBase, iSR);
    UARTIntEnable(uSBUARTBase, UART_INT_RX);
    UARTIntEnable(MIDUARTBase, UART_INT_RX);
    IntPrioritySet(UART_MAPPING[uSBUARTNum][UART_INT], 0b00100000);
    IntPrioritySet(UART_MAPPING[MIDUARTNum][UART_INT], 0b01100000);

    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        activeNotes[i]               =  0;
        volMode[i]               =  3;
        absFreq[i]               =  0.0f;
        singleNoteMaxDuty[i]     =  0.0f;
        singleNoteMaxOntimeUS[i] =  0.0f;
        coilPan[i]               = -1.0f;
        inversPanReach[i]        =  3.0f;

        // To prevent excessive copy operations when ordering the notes,
        // orderedNotes only contains the pointers to the actual Note objects.
        for (uint32_t j = 0; j < MAX_VOICES; j++)
        {
            orderedNotes[i][j] = &(notes[i][j]);
        }
    }

    // Assign all channels to all coils.
    uint32_t allCoils = (1 << COIL_COUNT) - 1;
    for (uint32_t i = 0; i < 16; i++)
    {
        channels[i].coils = allCoils;
    }

    // Init all channels
    for (uint32_t channel = 0; channel < 16; channel++)
    {
        channels[channel].resetControllers();
    }
}

void MIDI::usbUartISR()
{
    // Read and clear the asserted interrupts
    volatile uint32_t uartIntStatus;
    uartIntStatus = UARTIntStatus(uSBUARTBase, true);
    UARTIntClear(uSBUARTBase, uartIntStatus);

    // Store all available chars in bigger buffer.
    while (UARTCharsAvail(uSBUARTBase))
    {
        addData(UARTCharGet(uSBUARTBase));
    }
}

void MIDI::midiUartISR()
{
    // Read and clear the asserted interrupts
    volatile uint32_t uartIntStatus;
    uartIntStatus = UARTIntStatus(MIDUARTBase, true);
    UARTIntClear(MIDUARTBase, uartIntStatus);

    // Store all available chars in bigger buffer.
    while (UARTCharsAvail(MIDUARTBase))
    {
        addData(UARTCharGet(MIDUARTBase));
    }
}

void MIDI::addData(uint8_t data)
{
    UARTBuffer[UARTBufferWriteIndex++] = data;
    if (UARTBufferWriteIndex >= UARTBufferSize)
    {
        UARTBufferWriteIndex = 0;
    }
    if (UARTBufferWriteIndex == UARTBufferReadIndex)
    {
        UARTBufferReadIndex = UARTBufferWriteIndex + 1;
    }
}

void MIDI::newData(uint32_t c)
{
    // Minimum MIDI Data Processing
    if (c & 0b10000000) // The first byte of a MIDI Command starts with a 1. Following bytes start with a 0.
    {
        // Ignore System messages.
        if (0xf0 <= c && c <= 0xf7)
        {
            // System exclusive and system common messages reset the running status.
            dataIndex = 0;
            return;
        }
        else if (0xf8 <= c)
        {
            // System real time messages do not affect the running status.
            return;
        }
        else
        {
            // Lower 4 bits are channel.
            channel = c & 0x0f;
            data[0] = c;
            dataIndex = 1;
        }
    }
    else if (dataIndex >= 1) // Data byte can't be the first byte of the Command.
    {
        data[dataIndex++] = c;

        if (dataIndex >= 2)
        {
            // Check if any channel data is modified. Set the affected coils as modified afterwards.
            bool channelChange = false;

            switch (data[0] & 0xf0)
            {
            case 0x80: // Note off
                if (dataIndex >= 3)
                {
                    // Keep running status
                    dataIndex = 1;
                    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                    {
                        for (uint32_t note = 0; note < MAX_VOICES; note++)
                        {
                            if (note >= coilMaxVoices[coil])
                            {
                                break;
                            }
                            if (data[1] == notes[coil][note].number
                                    && channel == notes[coil][note].channel)
                            {
                                notes[coil][note].velocity = 0;
                                notes[coil][note].changed  = true;
                                break;
                            }
                        }
                    }
                }
                break;
            case 0x90: // Note on
                if (dataIndex >= 3)
                {
                    // Keep running status
                    dataIndex = 1;
                    if (data[2]) // Note has a velocity
                    {
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            if (channels[channel].coils & (1 << coil))
                            {
                                uint32_t targetNote = activeNotes[coil];
                                uint32_t maxCoilVoices = coilMaxVoices[coil];
                                bool foundNote = false;
                                for (uint32_t note = 0; note < MAX_VOICES; note++)
                                {
                                    if (note >= coilMaxVoices[coil])
                                    {
                                        break;
                                    }
                                    if (data[1] == orderedNotes[coil][note]->number
                                            && channel == orderedNotes[coil][note]->channel)
                                    {
                                        targetNote = note;
                                        foundNote = true;
                                        break;
                                    }
                                }
                                if (!foundNote)
                                {
                                    if (targetNote == maxCoilVoices)
                                    {
                                        targetNote = maxCoilVoices - 1;
                                        Note * tempNote = orderedNotes[coil][0];
                                        for (uint32_t note = 1; note < MAX_VOICES; note++)
                                        {
                                            if (note >= coilMaxVoices[coil])
                                            {
                                                break;
                                            }
                                            orderedNotes[coil][note - 1] = orderedNotes[coil][note];
                                        }
                                        // The other values will be overwritten anyway.
                                        orderedNotes[coil][maxCoilVoices - 1] = tempNote;
                                        orderedNotes[coil][maxCoilVoices - 1]->afterTouch   = 0;
                                        orderedNotes[coil][maxCoilVoices - 1]->rawOntimeUS  = 0.0f;
                                        orderedNotes[coil][maxCoilVoices - 1]->ADSROntimeUS = 0.0f;
                                    }
                                    else
                                    {
                                        targetNote = activeNotes[coil]++;
                                    }
                                }

                                orderedNotes[coil][targetNote]->number   = data[1];
                                orderedNotes[coil][targetNote]->velocity = data[2];
                                orderedNotes[coil][targetNote]->ADSRMode = 'A';
                                orderedNotes[coil][targetNote]->channel  = channel;
                                orderedNotes[coil][targetNote]->changed  = true;
                            }
                        }
                    }
                    else // Note has no velocity = note off. Code copy pasted from note off command.
                    {
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            for (uint32_t note = 0; note < MAX_VOICES; note++)
                            {
                                if (note >= coilMaxVoices[coil])
                                {
                                    break;
                                }
                                if (data[1] == notes[coil][note].number
                                        && channel == notes[coil][note].channel)
                                {
                                    notes[coil][note].velocity = 0;
                                    notes[coil][note].changed  = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            case 0xA0: // Polyphonic Aftertouch
                if (dataIndex >= 3)
                {
                    // Keep running status
                    dataIndex = 1;
                    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                    {
                        if (channels[channel].coils & (1 << coil))
                        {
                            for (uint32_t i = 0; i < MAX_VOICES; i++)
                            {
                                if (data[1] == notes[coil][i].number
                                        && channel == notes[coil][i].channel)
                                {
                                    notes[coil][i].afterTouch = data[2];
                                    notes[coil][i].changed = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            case 0xB0: // Control Change / Channel Mode
                if (dataIndex >= 3)
                {
                    // Keep running status
                    dataIndex = 1;
                    switch (data[1])
                    {
                    case 0x01: // Modulation Wheel
                        channels[channel].modulation = data[2] / 128.0f;
                        break;
                    case 0x02: // Breath Controller

                        break;
                    case 0x07: // Channel Volume
                        channels[channel].volume = data[2] / 128.0f;
                        channelChange = true;
                        break;
                    case 0x0A: // Pan
                        if (!channels[channel].notePanEnabled)
                        {
                            channels[channel].pan = data[2] / 128.0f;
                            channelChange = true;
                        }
                        break;
                    case 0x0B: // Expression coarse
                        channels[channel].expression = data[2] / 128.0f;
                        break;
                    case 0x40: // Sustain Pedal
                        if (data[2] >= 64)
                        {
                            channels[channel].sustainPedal = true;
                        }
                        else
                        {
                            channels[channel].sustainPedal = false;
                        }
                        channelChange = true;
                        break;
                    case 0x43: // Damper Pedal
                        if (data[2] >= 64)
                        {
                            channels[channel].damperPedal = true;
                        }
                        else
                        {
                            channels[channel].damperPedal = false;
                        }
                        channelChange = true;
                        break;
                    case 0x62: // Non Registered Parameter Number, fine
                        channels[channel].NRPN &= 0xff00;
                        channels[channel].NRPN += data[2];

                        // Can't receive RP and NRP data at the same time.
                        channels[channel].RPN   = 0x7f7f;
                        break;
                    case 0x63: // Non Registered Parameter Number, coarse
                        channels[channel].NRPN &= 0x00ff;
                        channels[channel].NRPN += (data[2] << 8);

                        // Can't receive RP and NRP data at the same time.
                        channels[channel].RPN   = 0x7f7f;
                        break;
                    case 0x64: // Registered Parameter Number, fine
                        channels[channel].RPN &= 0xff00;
                        channels[channel].RPN += data[2];

                        // Can't receive RP and NRP data at the same time.
                        channels[channel].NRPN = 0x7f7f;
                        break;
                    case 0x65: // Registered Parameter Number, coarse
                        channels[channel].RPN &= 0x00ff;
                        channels[channel].RPN += (data[2] << 8);

                        // Can't receive RP and NRP data at the same time.
                        channels[channel].NRPN = 0x7f7f;
                        break;
                    case 0x06: // (N)RPN Data Entry, coase
                        // Registered Parameter
                        if (channels[channel].RPN == 0) // Pitch bend range
                        {
                            channels[channel].pitchBendRangeCoarse = data[2];
                            channels[channel].pitchBendRange  =   channels[channel].pitchBendRangeCoarse
                                                                    + channels[channel].pitchBendRangeFine / 100.0f;
                            channels[channel].pitchBendRange /= 8192.0f;
                            channelChange = true;
                        }
                        else if (channels[channel].RPN == 1) // Fine tuning
                        {
                            channels[channel].fineTuningCoarse = data[2];
                            channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                            + channels[channel].fineTuningFine) - 8192.0f;
                            channels[channel].tuning /= 4096.0f;
                            channels[channel].tuning += channels[channel].coarseTuning;
                            channelChange = true;
                        }
                        else if (channels[channel].RPN == 2) // Coarse tuning
                        {
                            channels[channel].coarseTuning = data[2];
                            channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                            + channels[channel].fineTuningFine) - 8192.0f;
                            channels[channel].tuning /= 4096.0f;
                            channels[channel].tuning += channels[channel].coarseTuning;
                            channelChange = true;
                        }

                        // Non-Registered Parameter
                        if (channels[channel].NRPN == (42 << 8) + 1) // Note pan mode - source range upper limit
                        {
                            channels[channel].notePanSourceRangeHigh = data[2];
                            channelChange = true;
                        }
                        else if (channels[channel].NRPN == (42 << 8) + 2) // Note pan mode - target range upper limit
                        {
                            channels[channel].notePanTargetRangeHigh = data[2] / 128.0f;
                            channelChange = true;
                        }
                        break;
                    case 0x26: // (N)RPN Data Entry, fine
                        if (channels[channel].RPN == 0) // Pitch bend range
                        {
                            channels[channel].pitchBendRangeFine = data[2];
                            channels[channel].pitchBendRange  =   channels[channel].pitchBendRangeCoarse
                                                                    + channels[channel].pitchBendRangeFine / 100.0f;
                            channels[channel].pitchBendRange /= 8192.0f;
                            channelChange = true;
                        }
                        else if (channels[channel].RPN == 1) // Fine tuning
                        {
                            /*
                             * Fine tuning mapping is similar to pitch bend. A 14 bit value (0..16383) is mapped to -2.0f..2.0f
                             * Coarse tuning is unmapped.
                             */
                            channels[channel].fineTuningFine = data[2];
                            channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                            + channels[channel].fineTuningFine) - 8192.0f;
                            channels[channel].tuning /= 4096.0f;
                            channels[channel].tuning += channels[channel].coarseTuning;
                            channelChange = true;
                        }

                        // Non-Registered Parameter
                        if (channels[channel].NRPN == (42 << 8) + 0) // Note pan mode - enable/disable
                        {
                            if (data[2] == 2)
                            {
                                // Omni Mode (Note plays everywhere)
                                channels[channel].notePanOmniMode = true;
                            }
                            else
                            {
                                channels[channel].notePanOmniMode = false;
                                channels[channel].notePanEnabled = data[2];
                            }
                        }
                        else if (channels[channel].NRPN == (42 << 8) + 1) // Note pan mode - source range lower limit
                        {
                            channels[channel].notePanSourceRangeLow = data[2];
                            channelChange = true;
                        }
                        else if (channels[channel].NRPN == (42 << 8) + 2) // Note pan mode - target range lower limit
                        {
                            channels[channel].notePanTargetRangeLow = data[2] / 128.0f;
                            channelChange = true;
                        }
                        break;
                    case 0x78: // All Sounds off
                        channels[channel].sustainPedal = false;
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            if (channels[channel].coils & (1 << coil))
                            {
                                for (uint32_t note = 0; note < MAX_VOICES; note++)
                                {
                                    notes[coil][note].rawOntimeUS = 0;
                                }
                                coilChange[coil] = true;
                            }
                        }
                        break;
                    case 0x79: // Reset all Controllers
                        channels[channel].resetControllers();
                        break;
                    case 0x7B: // All Notes off
                        channels[channel].sustainPedal = false;
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            if (channels[channel].coils & (1 << coil))
                            {
                                for (uint32_t note = 0; note < MAX_VOICES; note++)
                                {
                                    notes[coil][note].ADSRMode = 'R';
                                }
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 0xE0: // Pitch Bend
                if (dataIndex >= 3)
                {
                    dataIndex = 1;
                    channels[channel].pitchBend = ((data[2] << 7) + data[1]);
                    channels[channel].pitchBend -= 8192.0f;
                    channelChange = true;
                }
                break;
            case 0xD0: // Channel Aftertouch
                dataIndex = 1;
                channels[channel].channelAfterTouch = data[1];
                channelChange = true;
                break;
            case 0xC0: // Program Change
                dataIndex = 1;
                if (data[1] <= ADSR_PROGRAM_COUNT)
                {
                    channels[channel].program = data[1];
                }
                else
                {
                    channels[channel].program = 0;
                }
                break;
            default:
                dataIndex = 0;
                break;
            }
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                if (channels[channel].coils & (1 << coil))
                {
                    coilChange[coil] |= channelChange;
                }
            }
        }
    }
}

void MIDI::UARTEnable(bool usbUart)
{
    if (usbUart)
    {
        UARTIntEnable(uSBUARTBase, UART_INT_RX);
    }
    else
    {
        UARTIntEnable(MIDUARTBase, UART_INT_RX);
    }
}

void MIDI::UARTDisable(bool usbUart)
{
    if (usbUart)
    {
        UARTIntDisable(uSBUARTBase, UART_INT_RX);
    }
    else
    {
        UARTIntDisable(MIDUARTBase, UART_INT_RX);
    }
}

void MIDI::start()
{
    for (uint32_t channel = 0; channel < 16; channel++)
    {
        channels[channel].resetControllers();
    }
    playing = true;
}

void MIDI::stop()
{
    playing = false;
}

void MIDI::setVolSettings(uint32_t coil, float ontimeUSMax, float dutyPermMax)
{
    singleNoteMaxOntimeUS[coil] = ontimeUSMax;
    singleNoteMaxDuty[coil]     = dutyPermMax / 1000.0f;

    // Prevent divide by 0.
    if (ontimeUSMax < 1.0f)
    {
        ontimeUSMax = 1.0f;
    }

    // Determine crossover note at which abs. and rel. mode would have equal frequency.
    absFreq[coil] = dutyPermMax / ontimeUSMax * 1000.0f;

    coilChange[coil] = true;
}

void MIDI::setChannels(uint32_t coil, uint32_t chns)
{
    for (uint32_t i = 0; i < 16; i++)
    {
        if (chns & (1 << i))
        {
            channels[i].coils |=  (1 << coil);
        }
        else
        {
            channels[i].coils &= ~(1 << coil);
        }
    }
}

void MIDI::setPan(uint32_t coil, uint32_t pan)
{
    if (pan < 128)
    {
        coilPan[coil] = pan / 128.0f;
    }
    else
    {
        coilPan[coil] = -1.0f;
    }
}

void MIDI::setPanReach(uint32_t coil, uint32_t reach)
{
    if (reach & 0b10000000)
    {
        panConstVol[coil] = true;
        reach &= 0b01111111;
    }
    else
    {
        panConstVol[coil] = false;
    }

    if (reach)
    {
        inversPanReach[coil] = 128.0f / reach;
    }
    else
    {
        // only needs to be >>128.0f
        inversPanReach[coil] = 1024.0f;
    }
}

void MIDI::setTotalMaxDutyPerm(uint32_t coil, float maxDuty)
{
    // dutyUS = ontime[us] * freq[Hz] = duty * 1000000 = (dutyPerm / 1000) * 1000000
    totalMaxDutyUS[coil] = maxDuty * 1000.0f;
}

void MIDI::setMaxVoices(uint32_t coil, uint32_t maxVoices)
{
    if (maxVoices > MAX_VOICES)
    {
        maxVoices = MAX_VOICES;
    }
    coilMaxVoices[coil] = maxVoices;
}

void MIDI::resetNRPs(uint32_t chns)
{
    /*
     *  The bits in chns indicate which channels shall reset their NRPs.
     *  A value equal or higher than 2^16-1 causes a reset on all channels.
     *  Default value is 0xffff (reset on all channels).
     */

    if (chns > 0xffff)
    {
        chns = 0xffff;
    }
    for (uint32_t i = 0; i < 16; i++)
    {
        if (chns & (1 << i))
        {
            channels[i].resetNRPs();
        }
    }
}

bool MIDI::isPlaying()
{
    return playing;
}

void MIDI::process()
{
    // Process all data that's in the buffer.
    while (UARTBufferReadIndex != UARTBufferWriteIndex)
    {
        newData(UARTBuffer[UARTBufferReadIndex++]);
        if (UARTBufferReadIndex >= UARTBufferSize)
        {
            UARTBufferReadIndex = 0;
        }
    }

    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        for (uint32_t noteNum = 0; noteNum < MAX_VOICES; noteNum++)
        {
            if (noteNum >= activeNotes[coil])
            {
                break;
            }
            if (coilChange[coil] || orderedNotes[coil][noteNum]->changed)
            {
                Note* note =  orderedNotes[coil][noteNum];
                Channel* channel = &(channels[note->channel]);

                note->changed = false;

                if (note->velocity)
                {
                    float noteNumFloat =   float(note->number)
                                         + channel->pitchBend * channel->pitchBendRange
                                         + channel->tuning;
                    note->frequency = exp2f((noteNumFloat - 69.0f) / 12.0f) * 440.0f;
                    note->periodUS = 1000000.0f / note->frequency;
                    note->periodTolUS = note->periodUS / 2;

                    // Determine MIDI volume, including all effects that are not time-dependant.
                    float vol = note->velocity / 128.0f;
                    if (channel->damperPedal)
                    {
                        vol *= 0.6f;
                    }

                    if (coilPan[coil] >= 0.0f && !channel->notePanOmniMode)
                    {
                        float pan = 0.0f;
                        if (channel->notePanEnabled)
                        {
                            /*
                             * In this mode the note number determines the pan position of the note.
                             * This is done with a mapping. A source range, convering any part of
                             * the [0..127] note number range, will be mapped to a target range,
                             * covering any part of the [0..1] pan position range.
                             */
                            if (noteNumFloat <= channel->notePanSourceRangeLow)
                            {
                                pan = channel->notePanTargetRangeLow;
                            }
                            else if (noteNumFloat >= channel->notePanSourceRangeHigh)
                            {
                                pan = channel->notePanTargetRangeHigh;
                            }
                            else
                            {
                                pan =   (noteNumFloat - channel->notePanSourceRangeLow)
                                      * (channel->notePanTargetRangeHigh - channel->notePanTargetRangeLow)
                                      / (channel->notePanSourceRangeHigh - channel->notePanSourceRangeLow)
                                      +  channel->notePanTargetRangeLow;
                            }
                        }
                        else
                        {
                            pan = channel->pan;
                        }

                        // 1.01f instead of 1.0f to include the borders of the range.
                        note->panVol = 1.01f - inversPanReach[coil] * fabsf(pan - coilPan[coil]);
                        if (note->panVol <= 0.0f)
                        {
                            note->panVol = 0.0f;
                        }
                        else if (panConstVol[coil])
                        {
                            note->panVol = 1.0f;
                        }

                    }
                    else
                    {
                        note->panVol = 1.0f;
                    }

                    vol *= channel->volume * channel->expression;

                    // Determine if note is played in absolute (= same maxOntime for all notes) mode
                    // or in relative mode (= same maxDuty for all notes)
                    if (note->frequency >= absFreq[coil])
                    {
                        note->rawOntimeUS = singleNoteMaxOntimeUS[coil] * vol;
                    }
                    else
                    {
                        note->rawOntimeUS = (1000000.0f / note->frequency) * singleNoteMaxDuty[coil] * vol;
                    }
                }
                else if (!channel->sustainPedal)
                {
                    note->ADSRMode = 'R';
                }
            }
        }
        coilChange[coil] = false;
    }
    updateEffects();
    removeDeadNotes();
}

float MIDI::getLFOVal(uint32_t channel)
{
    if (channels[channel].modulation)
    {
        /*
         *               /            t   \
         * LFO_SINE = sin| 2 * Pi * ----- |
         *               \           T_0  /
         *
         *       1   /  LFO_SINE + 1       ModWheelValue    \
         * val = - * | --------------- * ------------------ |
         *       2   \        2           MaxModWheelValue  /
         *
         * sine wave between 0 and 1 mapped to the desired modulation depth (50% max).
         */
        return (sinf(6.283185307179586f * float(sys.getSystemTimeUS()) / LFOPeriodUS) + 1) / 4.0f
                * channels[channel].modulation;
    }
    else
    {
        return 0.0f;
    }
}

void MIDI::updateEffects()
{
    // Needed for ADSR
    float targetAmp;
    float lastAmp;
    float inversDurationUS;
    float ampDiff;
    float currentTime = sys.getSystemTimeUS();
    if (currentTime < ADSRTimeUS)
    {
        ADSRTimeUS = currentTime;
    }
    float timeDiffUS = currentTime - ADSRTimeUS;
    if (timeDiffUS > effectResolutionUS)
    {
        ADSRTimeUS = currentTime;
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            float totalDutyUS = 0.0f;
            for (uint32_t note = 0; note < MAX_VOICES; note++)
            {
                if (note >= activeNotes[coil])
                {
                    break;
                }
                uint32_t program = channels[orderedNotes[coil][note]->channel].program;
                Note* currentNote = orderedNotes[coil][note];
                if (program)
                {
                    switch (currentNote->ADSRMode)
                    {
                    case 'A':
                        targetAmp = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_ATTACK_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_RELEASE_AMP]);
                        inversDurationUS  = (ADSR_PROGRAMS[program][ADSR_ATTACK_INVDUR_US]);
                        break;
                    case 'D':
                        targetAmp = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_DECAY_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_ATTACK_AMP]);
                        inversDurationUS  = (ADSR_PROGRAMS[program][ADSR_DECAY_INVDUR_US]);
                        break;
                    case 'S':
                        targetAmp = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_SUSTAIN_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_DECAY_AMP]);
                        inversDurationUS  = (ADSR_PROGRAMS[program][ADSR_SUSTAIN_INVDUR_US]);
                        break;
                    case 'R':
                        targetAmp = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_RELEASE_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (ADSR_PROGRAMS[program][ADSR_SUSTAIN_AMP]);
                        inversDurationUS  = (ADSR_PROGRAMS[program][ADSR_RELEASE_INVDUR_US]);
                        break;
                    }

                    ampDiff = targetAmp - lastAmp;

                    currentNote->ADSROntimeUS += ampDiff * timeDiffUS * inversDurationUS;
                    if (    (currentNote->ADSROntimeUS >= targetAmp && ampDiff >= 0)
                         || (currentNote->ADSROntimeUS <= targetAmp && ampDiff <= 0))
                    {
                        if (currentNote->ADSRMode == 'A')
                        {
                            currentNote->ADSRMode = 'D';
                        }
                        else if (currentNote->ADSRMode == 'D')
                        {
                            currentNote->ADSRMode = 'S';
                        }
                        currentNote->ADSROntimeUS = targetAmp;
                    }
                }
                else
                {
                    // No ADSR calculations. A/D/S = on, R = off
                    if (currentNote->ADSRMode != 'R')
                    {
                        // ADSRMode must not be 'A' otherwise it will not be removed by MIDI::removeDeadNotes
                        currentNote->ADSRMode = 'S';

                        currentNote->ADSROntimeUS = currentNote->rawOntimeUS;
                    }
                    else
                    {
                        currentNote->ADSROntimeUS = 0;
                    }
                }

                // After calculation of ADSR envelope, add other effects like modulation
                currentNote->finishedOntimeUS = currentNote->ADSROntimeUS
                                                * (1.0f - getLFOVal(currentNote->channel))
                                                * currentNote->panVol
                                                * playing;

                totalDutyUS += currentNote->finishedOntimeUS * currentNote->frequency;
            }

            if (totalDutyUS > totalMaxDutyUS[coil])
            {
                // Duty of all notes together exceeds coil limit; reduce ontimes.

                // Factor by which ontimes must be reduced.
                totalDutyUS = totalMaxDutyUS[coil] / totalDutyUS;
                for (uint32_t note = 0; note < activeNotes[coil]; note++)
                {
                    orderedNotes[coil][note]->finishedOntimeUS *= totalDutyUS;
                }
            }
        }
    }
}

void MIDI::removeDeadNotes()
{
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        uint32_t deadNotes = 0;
        for (uint32_t note = 0; note < MAX_VOICES; note++)
        {
            if (note >= activeNotes[coil])
            {
                break;
            }
            if (orderedNotes[coil][note]->ADSRMode != 'A' && orderedNotes[coil][note]->ADSROntimeUS < 0.1f)
            {
                deadNotes++;
                orderedNotes[coil][note]->number     = 0;
                orderedNotes[coil][note]->nextFireUS = 0;
            }
            else if (deadNotes)
            {
                Note *tempNote                       = orderedNotes[coil][note - deadNotes];
                orderedNotes[coil][note - deadNotes] = orderedNotes[coil][note];
                orderedNotes[coil][note]             = tempNote;
            }
        }
        activeNotes[coil] -= deadNotes;
    }
}
