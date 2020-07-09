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

void MIDI::init(System* sys, uint32_t usbUartNum, uint32_t baudRate, void (*usbISR)(void), uint32_t midiUartNum, void(*midiISR)(void))
{
    midiSys = sys;

    // Enable MIDI receiving over the USB UART (selectable baud rate) and a separate MIDI UART (31250 fixed baud rate).
    midiUSBUARTNum  = usbUartNum;
    midiMIDIUARTNum = midiUartNum;
    midiUSBUARTBase  = MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_BASE];
    midiMIDIUARTBase = MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_BASE];
    SysCtlPeripheralEnable(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_PORT_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_PORT_SYSCTL_PERIPH]);
    SysCtlDelay(3);
    GPIOPinConfigure(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_RX_PIN_CFG]);
    GPIOPinConfigure(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_RX_PIN_CFG]);
    GPIOPinConfigure(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_TX_PIN_CFG]);
    GPIOPinConfigure(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_TX_PIN_CFG]);
    GPIOPinTypeUART(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_PORT_BASE],
                    MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_TX_PIN] | MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_RX_PIN]);
    GPIOPinTypeUART(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_PORT_BASE],
                    MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_TX_PIN] | MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_RX_PIN]);

    UARTConfigSetExpClk(midiUSBUARTBase, midiSys->getClockFreq(), baudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTConfigSetExpClk(midiMIDIUARTBase, midiSys->getClockFreq(), 31250,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFODisable(midiUSBUARTBase);
    UARTFIFODisable(midiMIDIUARTBase);
    UARTIntRegister(midiUSBUARTBase,  usbISR);
    UARTIntRegister(midiMIDIUARTBase, midiISR);
    UARTIntEnable(midiUSBUARTBase,  UART_INT_RX);
    UARTIntEnable(midiMIDIUARTBase, UART_INT_RX);
    IntPrioritySet(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_INT],  0b00000000);
    IntPrioritySet(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_INT], 0b00100000);

    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        activeNotes[i]               =  0;
        midiVolMode[i]               =  3;
        midiAbsFreq[i]               =  0.0f;
        midiSingleNoteMaxDuty[i]     =  0.0f;
        midiSingleNoteMaxOntimeUS[i] =  0.0f;
        midiCoilPan[i]               = -1.0f;

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
        resetChannelControllers(channel);
    }
}

void MIDI::usbUartISR()
{
    // Read and clear the asserted interrupts
    volatile uint32_t uartIntStatus;
    uartIntStatus = UARTIntStatus(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_BASE], true);
    UARTIntClear(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_BASE], uartIntStatus);

    // Store all available chars in bigger buffer.
    while (UARTCharsAvail(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_BASE]))
    {
        addData(UARTCharGet(MIDI_UART_MAPPING[midiUSBUARTNum][MIDI_UART_BASE]));
    }
}

void MIDI::midiUartISR()
{
    // Read and clear the asserted interrupts
    volatile uint32_t uartIntStatus;
    uartIntStatus = UARTIntStatus(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_BASE], true);
    UARTIntClear(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_BASE], uartIntStatus);

    // Store all available chars in bigger buffer.
    while (UARTCharsAvail(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_BASE]))
    {
        addData(UARTCharGet(MIDI_UART_MAPPING[midiMIDIUARTNum][MIDI_UART_BASE]));
    }
}

void MIDI::addData(uint8_t data)
{
    midiUARTBuffer[midiUARTBufferWriteIndex++] = data;
    if (midiUARTBufferWriteIndex >= midiUARTBufferSize)
    {
        midiUARTBufferWriteIndex = 0;
    }
    if (midiUARTBufferWriteIndex == midiUARTBufferReadIndex)
    {
        midiUARTBufferReadIndex = midiUARTBufferWriteIndex + 1;
    }
}

void MIDI::newData(uint32_t c)
{
    // Minimum MIDI Data Processing
    if (c & 0b10000000) // The first byte of a MIDI Command starts with a 1. Following bytes start with a 0.
    {
        // Lower 4 bits are channel.
        midiChannel = c & 0x0f;
        midiData[0] = c;
        midiDataIndex = 1;
    }
    else if (midiDataIndex >= 1) // Data byte can't be the first byte of the Command.
    {
        midiData[midiDataIndex++] = c;

        if (midiDataIndex >= 2)
        {
            switch (midiData[0] & 0xf0)
            {
            case 0x80: // Note off
                if (midiDataIndex >= 3)
                {
                    midiDataIndex = 0;
                    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                    {
                        for (uint32_t i = 0; i < MAX_VOICES; i++)
                        {
                            if (midiData[1] == notes[coil][i].number
                                    && midiChannel == notes[coil][i].channel)
                            {
                                notes[coil][i].velocity = 0;
                                midiNoteChange = true;
                                break;
                            }
                        }
                    }
                }
                break;
            case 0x90: // Note on
                if (midiDataIndex >= 3)
                {
                    midiDataIndex = 0;
                    if (midiData[2]) // Note has a velocity
                    {
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            if (channels[midiChannel].coils & (1 << coil))
                            {
                                uint32_t targetNote = activeNotes[coil];
                                bool foundNote = false;
                                for (uint32_t i = 0; i < MAX_VOICES; i++)
                                {
                                    if (midiData[1] == orderedNotes[coil][i]->number
                                            && midiChannel == orderedNotes[coil][i]->channel)
                                    {
                                        targetNote = i;
                                        foundNote = true;
                                        break;
                                    }
                                }
                                if (!foundNote)
                                {
                                    if (targetNote == MAX_VOICES)
                                    {
                                        targetNote = MAX_VOICES - 1;
                                        Note * tempNote = orderedNotes[coil][0];
                                        for (uint32_t note = 1; note < MAX_VOICES; note++)
                                        {
                                            orderedNotes[coil][note - 1] = orderedNotes[coil][note];
                                        }
                                        // The other values will be overwritten anyway.
                                        orderedNotes[coil][MAX_VOICES - 1] = tempNote;
                                        orderedNotes[coil][MAX_VOICES - 1]->afterTouch   = 0;
                                        orderedNotes[coil][MAX_VOICES - 1]->rawOntimeUS  = 0.0f;
                                        orderedNotes[coil][MAX_VOICES - 1]->ADSROntimeUS = 0.0f;
                                    }
                                    else
                                    {
                                        targetNote = activeNotes[coil]++;
                                    }
                                }

                                orderedNotes[coil][targetNote]->number   = midiData[1];
                                orderedNotes[coil][targetNote]->velocity = midiData[2];
                                orderedNotes[coil][targetNote]->ADSRMode = 'A';
                                orderedNotes[coil][targetNote]->channel  = midiChannel;
                                midiNoteChange = true;
                            }
                        }
                    }
                    else // Note has no velocity = note off. Code copy pasted from note off command.
                    {
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            for (uint32_t note = 0; note < MAX_VOICES; note++)
                            {
                                if (midiData[1] == notes[coil][note].number
                                        && midiChannel == notes[coil][note].channel)
                                {
                                    notes[coil][note].velocity = 0;
                                    midiNoteChange = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            case 0xA0: // Polyphonic Aftertouch
                if (midiDataIndex >= 3)
                {
                    midiDataIndex = 0;
                    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                    {
                        if (channels[midiChannel].coils & (1 << coil))
                        {
                            for (uint32_t i = 0; i < MAX_VOICES; i++)
                            {
                                if (midiData[1] == notes[coil][i].number
                                        && midiChannel == notes[coil][i].channel)
                                {
                                    notes[coil][i].afterTouch = midiData[2];
                                    midiNoteChange = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            case 0xB0: // Control Change / Channel Mode
                if (midiDataIndex >= 3)
                {
                    midiDataIndex = 0;
                    switch (midiData[1])
                    {
                    case 0x01: // Modulation Wheel
                        channels[midiChannel].modulation = midiData[2] / 128.0f;
                        break;
                    case 0x02: // Breath Controller

                        break;
                    case 0x07: // Channel Volume
                        channels[midiChannel].volume = midiData[2] / 128.0f;
                        midiNoteChange = true;
                        break;
                    case 0x0A: // Pan
                        channels[midiChannel].pan = midiData[2] / 128.0f;
                        midiNoteChange = true;
                        break;
                    case 0x0B: // Expression coarse
                        channels[midiChannel].expression = midiData[2] / 128.0f;
                        break;
                    case 0x40: // Sustain Pedal
                        if (midiData[2] >= 64)
                        {
                            channels[midiChannel].sustainPedal = true;
                        }
                        else
                        {
                            channels[midiChannel].sustainPedal = false;
                        }
                        midiNoteChange = true;
                        break;
                    case 0x43: // Damper Pedal
                        if (midiData[2] >= 64)
                        {
                            channels[midiChannel].damperPedal = true;
                        }
                        else
                        {
                            channels[midiChannel].damperPedal = false;
                        }
                        midiNoteChange = true;
                        break;
                    case 0x64: // Registered Parameter Number, fine
                        midiRPN &= 0xff00;
                        midiRPN += midiData[2];
                        break;
                    case 0x65: // Registered Parameter Number, coarse
                        midiRPN &= 0x00ff;
                        midiRPN += (midiData[2] << 8);
                        break;
                    case 0x06: // RPN Data Entry, coase
                        if (midiRPN == 0) // Pitch bend range
                        {
                            channels[midiChannel].pitchBendRangeCoarse = midiData[2];
                            channels[midiChannel].pitchBendRange  =   channels[midiChannel].pitchBendRangeCoarse
                                                                    + channels[midiChannel].pitchBendRangeFine / 100.0f;
                            channels[midiChannel].pitchBendRange /= 8192.0f;
                            midiNoteChange = true;
                        }
                        else if (midiRPN == 1) // Fine tuning
                        {
                            channels[midiChannel].fineTuningCoarse = midiData[2];
                            channels[midiChannel].tuning = ((channels[midiChannel].fineTuningCoarse << 8)
                                                            + channels[midiChannel].fineTuningFine) - 8192.0f;
                            channels[midiChannel].tuning /= 4096.0f;
                            channels[midiChannel].tuning += channels[midiChannel].coarseTuning;
                            midiNoteChange = true;
                        }
                        else if (midiRPN == 2) // Coarse tuning
                        {
                            channels[midiChannel].coarseTuning = midiData[2];
                            channels[midiChannel].tuning = ((channels[midiChannel].fineTuningCoarse << 8)
                                                            + channels[midiChannel].fineTuningFine) - 8192.0f;
                            channels[midiChannel].tuning /= 4096.0f;
                            channels[midiChannel].tuning += channels[midiChannel].coarseTuning;
                            midiNoteChange = true;
                        }
                        break;
                    case 0x26: // RPN Data Entry, fine
                        if (midiRPN == 0) // Pitch bend range
                        {
                            channels[midiChannel].pitchBendRangeFine = midiData[2];
                            channels[midiChannel].pitchBendRange  =   channels[midiChannel].pitchBendRangeCoarse
                                                                    + channels[midiChannel].pitchBendRangeFine / 100.0f;
                            channels[midiChannel].pitchBendRange /= 8192.0f;
                            midiNoteChange = true;
                        }
                        else if (midiRPN == 1) // Fine tuning
                        {
                            /*
                             * Fine tuning mapping is similar to pitch bend. A 14 bit value (0..16383) is mapped to -2.0f..2.0f
                             * Coarse tuning is unmapped.
                             */
                            channels[midiChannel].fineTuningFine = midiData[2];
                            channels[midiChannel].tuning = ((channels[midiChannel].fineTuningCoarse << 8)
                                                            + channels[midiChannel].fineTuningFine) - 8192.0f;
                            channels[midiChannel].tuning /= 4096.0f;
                            channels[midiChannel].tuning += channels[midiChannel].coarseTuning;
                            midiNoteChange = true;
                        }
                        break;
                    case 0x78: // All Sounds off
                        channels[midiChannel].sustainPedal = false;
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            for (uint32_t note = 0; note < MAX_VOICES; note++)
                            {
                                notes[coil][note].rawOntimeUS = 0;
                                midiNoteChange = true;
                            }
                        }
                        break;
                    case 0x79: // Reset all Controllers
                        resetChannelControllers(midiChannel);
                        break;
                    case 0x7B: // All Notes off
                        channels[midiChannel].sustainPedal = false;
                        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                        {
                            for (uint32_t note = 0; note < MAX_VOICES; note++)
                            {
                                notes[coil][note].ADSRMode = 'R';
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 0xE0: // Pitch Bend
                if (midiDataIndex >= 3)
                {
                    midiDataIndex = 0;
                    channels[midiChannel].pitchBend = ((midiData[2] << 7) + midiData[1]);
                    channels[midiChannel].pitchBend -= 8192.0f;
                    midiNoteChange = true;
                }
                break;
            case 0xD0: // Channel Aftertouch
                midiDataIndex = 0;
                channels[midiChannel].channelAfterTouch = midiData[1];
                midiNoteChange = true;
                break;
            case 0xC0: // Program Change
                midiDataIndex = 0;
                if (midiData[1] <= MIDI_ADSR_PROGRAM_COUNT)
                {
                    channels[midiChannel].program = midiData[1];
                }
                else
                {
                    channels[midiChannel].program = 0;
                }
                break;
            default:
                midiDataIndex = 0;
                break;
            }
        }
    }
}

void MIDI::UARTEnable()
{
    UARTIntEnable(midiUSBUARTBase, UART_INT_RX);
}

void MIDI::UARTDisable()
{
    UARTIntDisable(midiUSBUARTBase, UART_INT_RX);
}

void MIDI::start()
{
    for (uint32_t channel = 0; channel < 16; channel++)
    {
        resetChannelControllers(channel);
    }
    midiPlaying = true;
}

void MIDI::stop()
{
    midiPlaying = false;
}

void MIDI::setVolSettings(uint32_t coil, float ontimeUSMax, float dutyPermMax, uint32_t volMode)
{
    midiSingleNoteMaxOntimeUS[coil] = ontimeUSMax;
    midiSingleNoteMaxDuty[coil] = dutyPermMax / 1000.0f;

    if (volMode == 1)
    {
        // Absolute Mode. Change to abs. mode at the very lowest note already.
        midiAbsFreq[coil] = 0.0f;
    }
    else if (volMode == 2)
    {
        // Relative Mode. Never change to abs. mode (= set crossover higher than highest note).
        midiAbsFreq[coil] = 40000.0f;
    }
    else if (volMode == 3)
    {
        // Auto Mode. Determine crossover note at which abs. and rel. mode would have equal frequency.
        midiAbsFreq[coil] = midiSingleNoteMaxDuty[coil] / midiSingleNoteMaxOntimeUS[coil] * 1000000.0f;
    }
    midiNoteChange = true;
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
        midiCoilPan[coil] = pan / 128.0f;
    }
    else
    {
        midiCoilPan[coil] = -1.0f;
    }
}

void MIDI::setTotalMaxDutyPerm(uint32_t coil, float maxDuty)
{
    // dutyUS = ontime[us] * freq[Hz] = duty * 1000000 = (dutyPerm / 1000) * 1000000
    midiTotalMaxDutyUS[coil] = maxDuty * 1000.0f;
}

bool MIDI::isPlaying()
{
    return midiPlaying;
}

void MIDI::process()
{
    // Process all data that's in the buffer.
    while (midiUARTBufferReadIndex != midiUARTBufferWriteIndex)
    {
        newData(midiUARTBuffer[midiUARTBufferReadIndex++]);
        if (midiUARTBufferReadIndex >= midiUARTBufferSize)
        {
            midiUARTBufferReadIndex = 0;
        }
    }

    if (midiNoteChange)
    {
        midiNoteChange = false;
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            for (uint32_t noteNum = 0; noteNum < MAX_VOICES; noteNum++)
            {
                if (notes[coil][noteNum].number)
                {
                    Note* note =  &(notes[coil][noteNum]);
                    Channel* channel = &(channels[note->channel]);

                    if (note->velocity)
                    {
                        float noteNumFloat =   float(note->number)
                                             + channel->pitchBend * channel->pitchBendRange
                                             + channel->tuning;
                        note->frequency = powf(2.0f, (noteNumFloat - 69.0f) / 12.0f) * 440.0f;
                        note->periodUS = 1000000.0f / note->frequency;
                        note->periodTolUS = note->periodUS / 2;

                        // Determine MIDI volume, including all effects that are not time-dependant.
                        float vol = note->velocity / 128.0f;
                        if (channel->damperPedal)
                        {
                            vol *= 0.6f;
                        }

                        if (midiCoilPan[coil] >= 0.0f)
                        {
                            vol *= 1.0f - fabsf(channel->pan - midiCoilPan[coil]);
                        }

                        vol *= channel->volume * channel->expression;

                        // Determine if note is played in absolute (= same maxOntime for all notes) mode
                        // or in relative mode (= same maxDuty for all notes)
                        if (note->frequency >= midiAbsFreq[coil])
                        {
                            note->rawOntimeUS = midiSingleNoteMaxOntimeUS[coil] * vol;
                        }
                        else
                        {
                            note->rawOntimeUS = (1000000.0f / note->frequency) * midiSingleNoteMaxDuty[coil] * vol;
                        }
                    }
                    else if (!channel->sustainPedal)
                    {
                        note->ADSRMode = 'R';
                    }
                }
            }
        }
    }
    updateEffects();
    removeDeadNotes();
}

void MIDI::resetChannelControllers(uint32_t channel)
{
    channels[channel].channelAfterTouch = 0;
    channels[channel].volume            = 1.0f;
    channels[channel].expression        = 1.0f;
    channels[channel].pitchBend         = 0.0f;
    channels[channel].modulation        = 0.0f;
    channels[channel].pan               = 0.5f;
    channels[channel].tuning            = 0.0f;
    channels[channel].pitchBendRange    = 2.0f / 8192.0f;
    //channels[channel].program           = MIDI_ADSR_PROGRAM_COUNT;
    channels[channel].sustainPedal      = false;
    channels[channel].damperPedal       = false;
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
         * sine wave between 0 and 0.5 mapped to the desired modulation depth.
         */
        return (sinf(6.283185307179586f * float(midiSys->getSystemTimeUS()) / midiLFOPeriodUS) + 1) / 4.0f
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
    float currentTime = midiSys->getSystemTimeUS();
    float timeDiffUS = currentTime - midiADSRTimeUS;
    midiADSRTimeUS = currentTime;
    if (timeDiffUS > 1.0f)
    {
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            float totalDutyUS = 0.0f;
            for (uint32_t note = 0; note < MAX_VOICES; note++)
            {
                if (note < activeNotes[coil])
                {
                    uint32_t program = channels[orderedNotes[coil][note]->channel].program;
                    Note* currentNote = orderedNotes[coil][note];
                    switch (currentNote->ADSRMode)
                    {
                    case 'A':
                        targetAmp = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_ATTACK_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_RELEASE_AMP]);
                        inversDurationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_ATTACK_INVDUR_US]);
                        break;
                    case 'D':
                        targetAmp = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_DECAY_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_ATTACK_AMP]);
                        inversDurationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_DECAY_INVDUR_US]);
                        break;
                    case 'S':
                        targetAmp = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_SUSTAIN_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_DECAY_AMP]);
                        inversDurationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_SUSTAIN_INVDUR_US]);
                        break;
                    case 'R':
                        targetAmp = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_RELEASE_AMP]);
                        lastAmp   = currentNote->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_SUSTAIN_AMP]);
                        inversDurationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_RELEASE_INVDUR_US]);
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

                    // After calculation of ADSR envelope, add other time-dependent effects like modulation
                    currentNote->finishedOntimeUS = currentNote->ADSROntimeUS
                                                    * (1.0f - getLFOVal(currentNote->channel))
                                                    * midiPlaying;

                    totalDutyUS += currentNote->finishedOntimeUS * currentNote->frequency;
                }

                if (totalDutyUS > midiTotalMaxDutyUS[coil])
                {
                    // Duty of all notes together exceeds coil limit; reduce ontimes.

                    // Factor by which ontimes must be reduced.
                    totalDutyUS = midiTotalMaxDutyUS[coil] / totalDutyUS;
                    for (uint32_t note = 0; note < activeNotes[coil]; note++)
                    {
                        orderedNotes[coil][note]->finishedOntimeUS *= totalDutyUS;
                    }
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
        for (uint32_t note = 0; note < activeNotes[coil]; note++)
        {
            if (orderedNotes[coil][note]->ADSRMode != 'A' && orderedNotes[coil][note]->ADSROntimeUS < 0.1f)
            {
                deadNotes++;
                orderedNotes[coil][note]->number = 0;
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
