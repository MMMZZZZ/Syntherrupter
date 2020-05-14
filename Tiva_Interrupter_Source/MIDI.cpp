/*
 * MIDI.cpp
 *
 *  Created on: 27.03.2020
 *      Author: Max
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

void MIDI::init(System* sys, uint32_t uartNum, uint32_t baudRate, void (*ISR)(void))
{
    midiSys = sys;

    // MIDI UART setup
    midiUARTNum = uartNum;
    midiUARTBase = MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_BASE];
    SysCtlPeripheralEnable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_PORT_SYSCTL_PERIPH]);
    SysCtlDelay(3);
    GPIOPinConfigure(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_RX_PIN_CFG]);
    GPIOPinConfigure(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_TX_PIN_CFG]);
    GPIOPinTypeUART(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_PORT_BASE],
                    MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_TX_PIN] | MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_RX_PIN]);

    UARTConfigSetExpClk(midiUARTBase, midiSys->getClockFreq(), baudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFODisable(midiUARTBase);
    UARTIntRegister(midiUARTBase, ISR);
    UARTIntEnable(midiUARTBase, UART_INT_RX);
    IntPrioritySet(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT], 0b00000000);

    for (uint32_t i = 0; i < COIL_COUNT; i++)
    {
        activeNotes[i]     = 0;
        midiVolMode[i]     = 3;
        midiAbsFreq[i]     = 0.0f;
        midiMaxDuty[i]     = 0.0f;
        midiMaxOntimeUS[i] = 0.0f;

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

    // Init all notes and channels
    resetAllValues();
}

void MIDI::newData(uint32_t c)
{
    uint32_t channel = 0;

    // Minimum MIDI Data Processing
    if (c & 0b10000000) // The first byte of a MIDI Command starts with a 1. Following bytes start with a 0.
    {
        // Lower 4 bits are channel.
        channel = c & 0x0f;
        // Check if any coil is assigned to this channel. If yes, process following data.
        if (channels[channel].coils)
        {
            midiISRData[0] = c;
            midiISRDataIndex = 1;
        }
    }
    else if (midiISRDataIndex) // Data byte can't be the first byte of the Command.
    {
        midiISRNewData = true;
        midiISRData[midiISRDataIndex++] = c;
    }
    switch (midiISRData[0] & 0xf0)
    {
    case 0x80: // Note off
        if (midiISRDataIndex >= 3)
        {
            midiISRDataIndex = 0;
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                for (uint32_t i = 0; i < activeNotes[coil]; i++)
                {
                    if (channel == orderedNotes[coil][i]->channel && midiISRData[1] == orderedNotes[coil][i]->number)
                    {
                        orderedNotes[coil][i]->ADSRMode = 'R';
                        break;
                    }
                }
            }
        }
        break;
    case 0x90: // Note on
        if (midiISRDataIndex >= 3)
        {
            midiISRDataIndex = 0;
            if (midiISRData[2]) // Note has a velocity
            {
                for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                {
                    if (channels[channel].coils && (1 << coil))
                    {
                        uint32_t targetNote = activeNotes[coil];
                        for (uint32_t i = 0; i < targetNote; i++)
                        {
                            if (midiISRData[1] == orderedNotes[coil][i]->number
                                    && channel == orderedNotes[coil][i]->channel)
                            {
                                targetNote = i;
                            }
                        }
                        if (targetNote >= MAX_VOICES)
                        {
                            targetNote = MAX_VOICES - 1;
                            volatile Note * tempNote = orderedNotes[coil][0];
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
                        if (targetNote >= activeNotes[coil])
                        {
                            activeNotes[coil] = targetNote + 1;
                        }

                        orderedNotes[coil][targetNote]->number = midiISRData[1];
                        orderedNotes[coil][targetNote]->velocity = midiISRData[2];
                        orderedNotes[coil][targetNote]->ADSRMode = 'A';
                    }
                }
            }
            else // Note has no velocity = note off. Code copy pasted from note off command.
            {
                for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                {
                    for (uint32_t note = 0; note < activeNotes[coil]; note++)
                    {
                        if (midiISRData[1] == orderedNotes[coil][note]->number
                                && channel == orderedNotes[coil][note]->channel)
                        {
                            orderedNotes[coil][note]->ADSRMode = 'R';
                            break;
                        }
                    }
                }
            }
        }
        break;
    case 0xA0: // Polyphonic Aftertouch
        if (midiISRDataIndex >= 3)
        {
            midiISRDataIndex = 0;
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                if (channels[channel].coils && (1 << coil))
                {
                    for (uint32_t i = 0; i < MAX_VOICES; i++)
                    {
                        if (midiISRData[1] == orderedNotes[coil][i]->number
                                && channel == orderedNotes[coil][i]->channel)
                        {
                            orderedNotes[coil][i]->afterTouch = midiISRData[2];
                        }
                    }
                }
            }
        }
        break;
    case 0xB0: // Control Change / Channel Mode
        if (midiISRDataIndex >= 3)
        {
            midiISRDataIndex = 0;
            switch (midiISRData[1])
            {
            case 0x01: // Modulation Wheel
                channels[channel].modulation = midiISRData[2];
                break;
            case 0x02: // Breath Controller

                break;
            case 0x07: // Channel Volume
                channels[channel].volume = midiISRData[2];
                break;
            case 0x0A: // Pan

                break;
            case 0x40: // Sustain Pedal
                if (midiISRData[2] >= 64)
                {
                    channels[channel].sustainPedal = true;
                }
                else
                {
                    channels[channel].sustainPedal = false;
                }
                break;
            case 0x43: // Damper Pedal
                if (midiISRData[2] >= 64)
                {
                    channels[channel].damperPedal = true;
                }
                else
                {
                    channels[channel].damperPedal = false;
                }
                break;
            case 0x78: // All Sounds off
                channels[channel].sustainPedal = false;
                for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
                {
                    for (uint32_t note = 0; note < MAX_VOICES; note++)
                    {
                        notes[coil][note].velocity = 0;
                    }
                }
                break;
            case 0x79: // Reset all Controllers
                resetAllValues();
                break;
            case 0x7B: // All Notes off
                channels[channel].sustainPedal = false;
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
        if (midiISRDataIndex >= 3)
        {
            midiISRDataIndex = 0;
            channels[channel].pitchBend = int32_t(((midiISRData[2] << 7) + midiISRData[1])) - 8192;
        }
        break;
    case 0xD0: // Channel Aftertouch
        if (midiISRDataIndex >= 2)
        {
            midiISRDataIndex = 0;
            channels[channel].channelAfterTouch = midiISRData[1];
        }
        break;
    case 0xC0: // Program Change
        if (midiISRDataIndex >= 2)
        {
            midiISRDataIndex = 0;
            channels[channel].program = midiISRData[1];
        }
        break;
    default:
        midiISRDataIndex = 0;
        break;
    }
}

void MIDI::enable()
{
    resetAllValues();

    // Enable UART RX Interrupt processing.
    IntEnable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT]);

    midiEnabled = true;
}

void MIDI::disable()
{
    // Disable and clear interrupts
    IntDisable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT]);

    uint32_t uartIntStatus = UARTIntStatus(midiUARTBase, true);
    UARTIntClear(midiUARTBase, uartIntStatus);

    midiEnabled = false;
}

void MIDI::play()
{
    midiPlaying = true;
}

void MIDI::stop()
{
    midiPlaying = false;
}

void MIDI::setVolSettings(uint32_t coil, float ontimeUSMax, float dutyPermMax, uint32_t volMode)
{
    midiMaxOntimeUS[coil] = ontimeUSMax;
    midiMaxDuty[coil] = dutyPermMax / 1000.0f;

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
        midiAbsFreq[coil] = midiMaxDuty[coil] / midiMaxOntimeUS[coil] * 1000000.0f;
    }
    midiISRNewData = true;
}

void MIDI::setChannels(uint32_t coil, uint32_t chns)
{
    for (uint32_t i = 0; i < 16; i++)
    {
        if (chns & (1 << i))
        {
            channels[i].coils |= (1 << coil);
        }
        else
        {
            channels[i].coils &= ~(1 << coil);
        }
    }
}

bool MIDI::isEnabled()
{
    return midiEnabled;
}

void MIDI::process()
{
    if (midiISRNewData)
    {
        IntDisable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT]);
        midiISRNewData = false;
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            for (uint32_t note = 0; note < MAX_VOICES; note++)
            {
                if (notes[coil][note].number)
                {
                    if (notes[coil][note].velocity)
                    {
                        float noteNumFloat = float(notes[coil][note].number) + 2.0f * float(channels[notes[coil][note].channel].pitchBend) / 8192.0f;
                        notes[coil][note].frequency = powf(2.0f, (noteNumFloat - 69.0f) / 12.0f) * 440.0f;
                        notes[coil][note].periodUS = lroundf(1000000.0f / notes[coil][note].frequency);
                        notes[coil][note].halfPeriodUS = notes[coil][note].periodUS / 2;

                        float vol = notes[coil][note].velocity / 127.0f;

                        // Determine if note is played in absolute (= same maxOntime for all notes) mode
                        // or in relative mode (= same maxDuty for all notes)
                        if (notes[coil][note].frequency >= midiAbsFreq[coil])
                        {
                            notes[coil][note].rawOntimeUS = midiMaxOntimeUS[coil] * vol;
                        }
                        else
                        {
                            notes[coil][note].rawOntimeUS = (1000000.0f / notes[coil][note].frequency) * midiMaxDuty[coil] * vol;
                        }
                    }
                    else if (midiADSREnabled)
                    {
                        notes[coil][note].ADSRMode = 'R';
                    }
                    else
                    {
                        notes[coil][note].rawOntimeUS  = 0.0f;
                        notes[coil][note].ADSROntimeUS = 0.0f;
                    }
                }
            }
        }
        IntEnable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT]);
    }
    updateEffects();
    removeDeadNotes();



    /*
     * Take data snapshot to prevent changes during calculations.
     *//*
    if (midiISRNewData)
    {
        IntDisable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT]);
        midiISRNewData     = false;
        midiADSREnabled    = midiISRADSREnabled;
        midiSustainPedal   = midiISRSustainPedal;
        midiPitchBend      = midiISRPitchBend;
        midiAfterTouch     = midiISRAfterTouch;
        midiModWheel       = midiISRModWheel;
        midiNoteAfterTouch = midiISRNoteAfterTouch;
        midiNoteNum        = midiISRNoteNum;
        midiNoteVel        = midiISRNoteVel;
        midiProgram        = midiISRProgram;
        if (midiISRNoteNew)
        {
            midiISRNoteNew = false;
            midiADSRMode = 'A';
            if (!midiISRSustainPedal)
            {
                midiADSROntimeUS = 0.0f;
            }
        }
        if (midiISRDamperPedal)
        {
            midiChnVol = midiISRChnVol * 0.6;
        }
        else
        {
            midiChnVol = midiISRChnVol;
        }
        IntEnable(MIDI_UART_MAPPING[midiUARTNum][MIDI_UART_INT]);
    }

    *//*
     * If sustain pedal is pushed keep highest Note velocity (ignore release
     *//*
    if (midiSustainPedal)
    {
        if (midiNoteVel > midiNoteVelPedal)
        {
            midiNoteVelPedal = midiNoteVel;
        }
    }
    else
    {
        midiNoteVelPedal = midiNoteVel;
    }
    if (midiNoteVelPedal && midiNoteNum >= 21)
    {
        float note = float(midiNoteNum) + 2.0 * float(midiPitchBend) / 8192.0;
        midiFrequency = powf(2.0, (note - 69.0) / 12.0) * 440.0;

        float vol = (float(midiNoteVelPedal) / 127.0f);

        // Determine if note is played in absolute (= same maxOntime for all notes) mode
        // or in relative mode (= same maxDuty for all notes)
        if (midiFrequency >= midiAbsFreq)
        {
            midiOntimeUS = midiOntimeUSMax * vol;
        }
        else
        {
            midiOntimeUS = (1000000.0f / midiFrequency) * midiDutyMax * vol;
        }
    }
    else if (midiADSREnabled)
    {
        midiADSRMode = 'R';
    }
    else
    {
        midiOntimeUS = 0.0f;
    }
    if (midiADSREnabled)
    {
        updateADSR();
    }
    else
    {
        midiADSROntimeUS = midiOntimeUS;
    }*/
}

/*
float MIDI::getOntimeUS()
{
    return midiADSROntimeUS * (1.0f - getLFOVal()) * float(midiChnVol) / 127.0f * midiPlaying;
}

float MIDI::getFrequency()
{
    return midiFrequency;
}
*/
void MIDI::resetAllValues()
{
    // Reset all values to default
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        for (uint32_t note = 0; note < MAX_VOICES; note++)
        {
            notes[coil][note].number       = 0;
            notes[coil][note].velocity     = 0;
            notes[coil][note].afterTouch   = 0;
            notes[coil][note].halfPeriodUS = 0;
            notes[coil][note].periodUS     = 0;
            notes[coil][note].ADSRMode     = 'A';
            notes[coil][note].frequency    = 0.0f;
            notes[coil][note].rawOntimeUS  = 0.0f;
            notes[coil][note].ADSROntimeUS = 0.0f;
        }
        activeNotes[coil] = 0;
    }
    for (uint32_t channel = 0; channel < 16; channel++)
    {
        channels[channel].channelAfterTouch = 0;
        channels[channel].modulation        = 0;
        channels[channel].pitchBend         = 0;
        channels[channel].program           = MIDI_ADSR_PROGRAM_COUNT;
        channels[channel].volume            = 127;
        channels[channel].sustainPedal      = false;
        channels[channel].damperPedal       = false;
    }


    /*midiADSRTimeUS = midiSys->getSystemTimeUS();

    midiFrequency      =   0;
    midiOntimeUS       =   0;

    midiISRNewData         = true;
    midiISRADSREnabled     = false;
    midiISRSustainPedal    = false;
    midiISRDamperPedal     = false;
    midiISRNoteNew         = true;
    midiISRPitchBend       = 0;
    midiISRAfterTouch      = 0;
    midiISRChnVol          = 127;
    midiISRDataIndex       = 0;
    midiISRModWheel        = 0;
    midiISRNoteAfterTouch  = 0;
    midiISRNoteNum         = 0;
    midiISRNoteVel         = 0;
    midiISRProgram         = 0;
    */
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
         *       /  LFO_SINE + 1       ModWheelValue    \
         * val = | --------------- * ------------------ |
         *       \        2           MaxModWheelValue  /
         *
         * sine wave between 0 and 1 mapped to the desired modulation depth.
         */
        return (sinf(float(midiSys->getSystemTimeUS()) * 6.283185307179586f / midiLFOPeriodUS) + 1)
                / 2.0f * (float(channels[channel].modulation) / 127.0f);
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
    float durationUS;
    float ampDiff;
    float currentTime = midiSys->getSystemTimeUS();
    float timeDiffUS = currentTime - midiADSRTimeUS;
    midiADSRTimeUS = currentTime;
    if (timeDiffUS > 1.0f)
    {
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            for (uint32_t note = 0; note < activeNotes[coil]; note++)
            {
                uint32_t program = channels[orderedNotes[coil][note]->channel].program;
                switch (orderedNotes[coil][note]->ADSRMode)
                {
                case 'A':
                    targetAmp = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_ATTACK_AMP]);
                    lastAmp   = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_RELEASE_AMP]);
                    durationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_ATTACK_DUR_US]);
                    break;
                case 'D':
                    targetAmp = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_DECAY_AMP]);
                    lastAmp   = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_ATTACK_AMP]);
                    durationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_DECAY_DUR_US]);
                    break;
                case 'S':
                    targetAmp = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_SUSTAIN_AMP]);
                    lastAmp   = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_DECAY_AMP]);
                    durationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_SUSTAIN_DUR_US]);
                    break;
                case 'R':
                    targetAmp = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_RELEASE_AMP]);
                    lastAmp   = orderedNotes[coil][note]->rawOntimeUS * (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_SUSTAIN_AMP]);
                    durationUS  = (MIDI_ADSR_PROGRAMS[program][MIDI_ADSR_RELEASE_DUR_US]);
                    break;
                }

                ampDiff = targetAmp - lastAmp;

                orderedNotes[coil][note]->ADSROntimeUS += ampDiff * timeDiffUS / durationUS;
                if (    (orderedNotes[coil][note]->ADSROntimeUS >= targetAmp && ampDiff >= 0)
                     || (orderedNotes[coil][note]->ADSROntimeUS <= targetAmp && ampDiff <= 0))
                {
                    if (orderedNotes[coil][note]->ADSRMode == 'A')
                    {
                        orderedNotes[coil][note]->ADSRMode = 'D';
                    }
                    else if (orderedNotes[coil][note]->ADSRMode == 'D')
                    {
                        orderedNotes[coil][note]->ADSRMode = 'S';
                    }
                    orderedNotes[coil][note]->ADSROntimeUS = targetAmp;
                }

                // After calculation of ADSR envelope, add other effects like channel volume or modulation
                orderedNotes[coil][note]->finishedOntimeUS = orderedNotes[coil][note]->ADSROntimeUS
                                                             * (1.0f - getLFOVal(orderedNotes[coil][note]->channel))
                                                             * float(channels[orderedNotes[coil][note]->channel].volume) / 127.0f
                                                             * midiPlaying;
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
            if (orderedNotes[coil][note]->ADSRMode != 'A' && orderedNotes[coil][note]->finishedOntimeUS < 0.1f)
            {
                deadNotes++;
                orderedNotes[coil][note]->number       = 0;
                orderedNotes[coil][note]->velocity     = 0;
                orderedNotes[coil][note]->frequency    = 0.0f;
                orderedNotes[coil][note]->halfPeriodUS = 0.0f;
                orderedNotes[coil][note]->periodUS     = 0.0f;
                orderedNotes[coil][note]->rawOntimeUS  = 0.0f;
                orderedNotes[coil][note]->ADSROntimeUS = 0.0f;
            }
            else if (deadNotes)
            {
                volatile Note *tempNote              = orderedNotes[coil][note - deadNotes];
                orderedNotes[coil][note - deadNotes] = orderedNotes[coil][note];
                orderedNotes[coil][note]             = tempNote;
            }
        }
        activeNotes[coil] -= deadNotes;
    }
}
