/*
 * MIDI.cpp
 *
 *  Created on: 27.03.2020
 *      Author: Max Zuidberg
 */

#include <MIDI.h>


UART                  MIDI::usbUart;
UART                  MIDI::midiUart;
ByteBuffer            MIDI::otherBuffer;
constexpr ByteBuffer* MIDI::BUFFER_LIST[];
uint8_t               MIDI::bufferMidiStatus[] = {0, 0, 0};
Channel               MIDI::channels[];
Note                  MIDI::unorderedNotes[];
Note*                 MIDI::notes[];
uint32_t              MIDI::notesCount         = 0;
uint32_t              MIDI::dataBytes          = 0;
bool                  MIDI::playing            = false;
float                 MIDI::ADSRTimeUS;
MIDIProgram           MIDI::programs[MAX_PROGRAMS];
constexpr float       MIDI::ADSR_LEGACY_PROGRAMS[MIDI::ADSR_PROGRAM_COUNT + 1][8];


MIDI::MIDI()
{
    // TODO Auto-generated constructor stub

}

MIDI::~MIDI()
{
    // TODO Auto-generated destructor stub
}

void MIDI::init(uint32_t usbBaudRate, void (*usbISR)(void), uint32_t midiUartPort, uint32_t midiUartRx, uint32_t midiUartTx, void(*midiISR)(void))
{
    // Enable MIDI receiving over the USB UART (selectable baud rate) and a separate MIDI UART (31250 fixed baud rate).
    usbUart.init(0, usbBaudRate, usbISR, 0b00100000);
    midiUart.init(midiUartPort, midiUartRx, midiUartTx, 31250, midiISR, 0b01100000);
    otherBuffer.init(128);

    // Initialize static class variables
    for (uint32_t note = 0; note < MAX_NOTES_COUNT; note++)
    {
        notes[note] = &(unorderedNotes[note]);
    }

    for (uint32_t prog = 0; prog < ADSR_PROGRAM_COUNT + 1; prog++)
    {
        for (uint32_t datapnt = 0; datapnt < 4; datapnt++)
        {
            programs[prog].setMode(MIDIProgram::Mode::lin);
            programs[prog].setDataPoint(datapnt, ADSR_LEGACY_PROGRAMS[prog][datapnt*2], 1.0f / ADSR_LEGACY_PROGRAMS[prog][datapnt*2+1]);
        }
    }
}

void MIDI::setCoilsToneList(ToneList* tonelist)
{
    this->tonelist = tonelist;
}

uint32_t MIDI::getNoteIndex(uint32_t channel, uint32_t noteNum)
{
    uint32_t noteIndex = MAX_NOTES_COUNT;
    if (channels[channel].coils)
    {
        for (uint32_t note = 0; note < MAX_NOTES_COUNT; note++)
        {
            if (note > notesCount)
            {
                break;
            }
            if (noteNum == notes[note]->number && channel == notes[note]->channel)
            {
                noteIndex = note;
                break;
            }
        }
    }
    return noteIndex;
}

bool MIDI::processBuffer(uint32_t b)
{
    /*
     * Minimum MIDI data processing.
     * Returns if valid data has been found.
     */

    ByteBuffer* buffer = BUFFER_LIST[b];

    static uint32_t channel{0};

    uint8_t c1 = buffer->read();

    if (c1 & 0b10000000) // The first byte of a MIDI Command starts with a 1. Following bytes start with a 0.
    {
        if (0xf8 <= c1)
        {
            // System real time messages do not affect the running status.
        }
        else
        {
            // Lower 4 bits are channel.
            channel = c1 & 0x0f;

            // Save running status
            bufferMidiStatus[b] = c1;
        }
        dataBytes = 0;
    }
    else
    {
        dataBytes++;

    }

    switch (bufferMidiStatus[b] & 0xf0)
    {
        case 0x80: // Note off
        {
            if (dataBytes == 1)
            {
                uint32_t noteIndex = getNoteIndex(channel, c1);
                if (noteIndex < MAX_NOTES_COUNT)
                {
                    notes[noteIndex]->velocity = 0;
                    notes[noteIndex]->changed  = true;
                }
            }
            break;
        }
        case 0x90: // Note on
        {
            static uint32_t noteIndex{MAX_NOTES_COUNT};
            static uint8_t  number{0};
            if (dataBytes == 1)
            {
                number = c1;
                noteIndex = getNoteIndex(channel, c1);
            }
            else if (dataBytes == 2)
            {
                if (c1) // Note has a velocity
                {
                    if (noteIndex >= MAX_NOTES_COUNT)
                    {
                        if (notesCount < MAX_NOTES_COUNT)
                        {
                            noteIndex = notesCount++;
                        }
                        else
                        {
                            noteIndex = MAX_NOTES_COUNT - 1;
                            Note * tempNote = notes[0];
                            for (uint32_t note = 1; note < MAX_NOTES_COUNT; note++)
                            {
                                notes[note - 1] = notes[note];
                            }
                            // The other values will be overwritten anyway.
                            notes[noteIndex] = tempNote;
                            notes[noteIndex]->afterTouch = 0;
                            notes[noteIndex]->rawVolume  = 0.0f;
                            notes[noteIndex]->ADSRVolume = 0.0f;
                        }
                    }

                    notes[noteIndex]->number     = number;
                    notes[noteIndex]->velocity   = c1;
                    notes[noteIndex]->ADSRStep   = 0;
                    notes[noteIndex]->ADSRTimeUS = 0.0f;
                    notes[noteIndex]->channel    = channel;
                    notes[noteIndex]->changed    = true;
                }
                else if (noteIndex >= MAX_NOTES_COUNT)// Note has no velocity = note off. Code copy pasted from note off command.
                {
                    notes[noteIndex]->velocity = 0;
                    notes[noteIndex]->changed  = true;
                }
            }
            break;
        }
        case 0xA0: // Polyphonic Aftertouch
        {
            static uint32_t noteIndex{MAX_NOTES_COUNT};
            if (dataBytes == 1)
            {
                noteIndex = getNoteIndex(channel, c1);
            }
            else if (dataBytes == 2)
            {
                notes[noteIndex]->afterTouch = c1;
                notes[noteIndex]->changed = true;
            }
            break;
        }
        case 0xB0: // Control Change / Channel Mode
        {
            static uint32_t controller{128};
            if (dataBytes == 1)
            {
                controller = c1;
            }
            else if (dataBytes == 2)
            {
                switch (controller)
                {
                case 0x01: // Modulation Wheel
                    channels[channel].modulation = c1 / 128.0f;
                    break;
                case 0x02: // Breath Controller

                    break;
                case 0x07: // Channel Volume
                    channels[channel].volume = c1 / 128.0f;
                    channels[channel].changed = true;
                    break;
                case 0x0A: // Pan
                    if (!channels[channel].notePanEnabled)
                    {
                        channels[channel].pan = c1 / 128.0f;
                        channels[channel].changed = true;
                    }
                    break;
                case 0x0B: // Expression coarse
                    channels[channel].expression = c1 / 128.0f;
                    break;
                case 0x40: // Sustain Pedal
                    if (c1 >= 64)
                    {
                        channels[channel].sustainPedal = true;
                    }
                    else
                    {
                        channels[channel].sustainPedal = false;
                    }
                    channels[channel].changed = true;
                    break;
                case 0x43: // Damper Pedal
                    if (c1 >= 64)
                    {
                        channels[channel].damperPedal = true;
                    }
                    else
                    {
                        channels[channel].damperPedal = false;
                    }
                    channels[channel].changed = true;
                    break;
                case 0x62: // Non Registered Parameter Number, fine
                    channels[channel].NRPN &= 0xff00;
                    channels[channel].NRPN += c1;

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].RPN   = 0x7f7f;
                    break;
                case 0x63: // Non Registered Parameter Number, coarse
                    channels[channel].NRPN &= 0x00ff;
                    channels[channel].NRPN += (c1 << 8);

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].RPN   = 0x7f7f;
                    break;
                case 0x64: // Registered Parameter Number, fine
                    channels[channel].RPN &= 0xff00;
                    channels[channel].RPN += c1;

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].NRPN = 0x7f7f;
                    break;
                case 0x65: // Registered Parameter Number, coarse
                    channels[channel].RPN &= 0x00ff;
                    channels[channel].RPN += (c1 << 8);

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].NRPN = 0x7f7f;
                    break;
                case 0x06: // (N)RPN Data Entry, coase
                    // Registered Parameter
                    if (channels[channel].RPN == 0) // Pitch bend range
                    {
                        channels[channel].pitchBendRangeCoarse = c1;
                        channels[channel].pitchBendRange  =   channels[channel].pitchBendRangeCoarse
                                                                + channels[channel].pitchBendRangeFine / 100.0f;
                        channels[channel].pitchBendRange /= 8192.0f;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].RPN == 1) // Fine tuning
                    {
                        channels[channel].fineTuningCoarse = c1;
                        channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                        + channels[channel].fineTuningFine) - 8192.0f;
                        channels[channel].tuning /= 4096.0f;
                        channels[channel].tuning += channels[channel].coarseTuning;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].RPN == 2) // Coarse tuning
                    {
                        channels[channel].coarseTuning = c1;
                        channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                        + channels[channel].fineTuningFine) - 8192.0f;
                        channels[channel].tuning /= 4096.0f;
                        channels[channel].tuning += channels[channel].coarseTuning;
                        channels[channel].changed = true;
                    }

                    // Non-Registered Parameter
                    if (channels[channel].NRPN == (42 << 8) + 1) // Note pan mode - source range upper limit
                    {
                        channels[channel].notePanSourceRangeHigh = c1;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].NRPN == (42 << 8) + 2) // Note pan mode - target range upper limit
                    {
                        channels[channel].notePanTargetRangeHigh = c1 / 128.0f;
                        channels[channel].changed = true;
                    }
                    break;
                case 0x26: // (N)RPN Data Entry, fine
                    if (channels[channel].RPN == 0) // Pitch bend range
                    {
                        channels[channel].pitchBendRangeFine = c1;
                        channels[channel].pitchBendRange  =   channels[channel].pitchBendRangeCoarse
                                                                + channels[channel].pitchBendRangeFine / 100.0f;
                        channels[channel].pitchBendRange /= 8192.0f;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].RPN == 1) // Fine tuning
                    {
                        /*
                         * Fine tuning mapping is similar to pitch bend. A 14 bit value (0..16383) is mapped to -2.0f..2.0f
                         * Coarse tuning is unmapped.
                         */
                        channels[channel].fineTuningFine = c1;
                        channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                        + channels[channel].fineTuningFine) - 8192.0f;
                        channels[channel].tuning /= 4096.0f;
                        channels[channel].tuning += channels[channel].coarseTuning;
                        channels[channel].changed = true;
                    }

                    // Non-Registered Parameter
                    if (channels[channel].NRPN == (42 << 8) + 0) // Note pan mode - enable/disable
                    {
                        if (c1 == 2)
                        {
                            // Omni Mode (Note plays everywhere)
                            channels[channel].notePanOmniMode = true;
                        }
                        else
                        {
                            channels[channel].notePanOmniMode = false;
                            channels[channel].notePanEnabled = c1;
                        }
                    }
                    else if (channels[channel].NRPN == (42 << 8) + 1) // Note pan mode - source range lower limit
                    {
                        channels[channel].notePanSourceRangeLow = c1;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].NRPN == (42 << 8) + 2) // Note pan mode - target range lower limit
                    {
                        channels[channel].notePanTargetRangeLow = c1 / 128.0f;
                        channels[channel].changed = true;
                    }
                    break;
                case 0x78: // All Sounds off
                    if (channels[channel].coils)
                    {
                        for (uint32_t note = 0; note < MAX_NOTES_COUNT; note++)
                        {
                            if (unorderedNotes[note].channel == channel)
                            {
                                unorderedNotes[note].number = 128;
                            }
                        }
                    }
                    break;
                case 0x79: // Reset all Controllers
                    channels[channel].resetControllers();
                    break;
                case 0x7B: // All Notes off
                    if (channels[channel].coils)
                    {
                        for (uint32_t note = 0; note < MAX_NOTES_COUNT; note++)
                        {
                            if (unorderedNotes[note].channel == channel)
                            {
                                unorderedNotes[note].ADSRStep = MIDIProgram::DATA_POINTS - 1;
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
        case 0xE0: // Pitch Bend
        {
            static float pb{0.0f};
            if (dataBytes == 1)
            {
                pb = c1 - 8192.0f;
            }
            else if (dataBytes == 2)
            {
                channels[channel].pitchBend = c1 * 128.0f + pb;
                channels[channel].changed = true;
            }
            break;
        }
        case 0xD0: // Channel Aftertouch
        {
            if (dataBytes == 1)
            {
                channels[channel].channelAfterTouch = c1;
                channels[channel].changed = true;
            }
            break;
        }
        case 0xC0: // Program Change
        {
            if (dataBytes == 1)
            {
                if (c1 <= ADSR_PROGRAM_COUNT)
                {
                    channels[channel].program = c1;
                }
                else
                {
                    channels[channel].program = 0;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return true;
}

void MIDI::start()
{
    if (!playing)
    {
        for (uint32_t channel = 0; channel < 16; channel++)
        {
            channels[channel].resetControllers();
        }
        playing = true;
    }
}

void MIDI::stop()
{
    playing = false;
    for (uint32_t noteNum = 0; noteNum < MAX_NOTES_COUNT; noteNum++)
    {
        if (noteNum >= notesCount)
        {
            break;
        }
        Note* note = notes[noteNum];
        note->number = 128;
    }
}

void MIDI::setVolSettings(float ontimeUSMax, float dutyPermMax)
{
    singleNoteMaxOntimeUS = ontimeUSMax;
    singleNoteMaxDuty     = dutyPermMax / 1000.0f;

    // Prevent divide by 0.
    if (ontimeUSMax < 1.0f)
    {
        ontimeUSMax = 1.0f;
    }

    // Determine crossover note at which abs. and rel. mode would have equal frequency.
    absFreq = dutyPermMax / ontimeUSMax * 1000.0f;
}

void MIDI::setChannels(uint32_t chns)
{
    activeChannels = chns;
    for (uint32_t i = 0; i < 16; i++)
    {
        if (chns & (1 << i))
        {
            channels[i].coils |=  (1 << coilNum);
        }
        else
        {
            channels[i].coils &= ~(1 << coilNum);
        }
    }
}

void MIDI::setPan(uint32_t pan)
{
    if (pan < 128)
    {
        coilPan = pan / 128.0f;
    }
    else
    {
        coilPan = -1.0f;
    }
    coilPanChanged = true;
}

void MIDI::setPanReach(uint32_t reach)
{
    if (reach & 0b10000000)
    {
        panConstVol = true;
        reach &= 0b01111111;
    }
    else
    {
        panConstVol = false;
    }

    if (reach)
    {
        inversPanReach = 128.0f / reach;
    }
    else
    {
        // only needs to be >>128.0f
        inversPanReach = 1024.0f;
    }
    coilPanChanged = true;
}

void MIDI::setMaxVoices(uint32_t maxVoices)
{
    if (maxVoices > MAX_VOICES)
    {
        maxVoices = MAX_VOICES;
    }
    coilMaxVoices = maxVoices;
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
            channels[i].changed = true;
        }
    }
}

bool MIDI::isPlaying()
{
    return playing;
}

void MIDI::process()
{
    if (playing)
    {
        // Process all data that's in the buffer.

        for (uint32_t bufferNum = 0; bufferNum < BUFFER_COUNT; bufferNum++)
        {
            while (BUFFER_LIST[bufferNum]->level()) // @suppress("Invalid arguments")
            {
                processBuffer(bufferNum);
            }
        }

        for (uint32_t noteNum = 0; noteNum < MAX_NOTES_COUNT; noteNum++)
        {
            if (noteNum >= notesCount)
            {
                break;
            }
            Note* note =  notes[noteNum];
            Channel* channel = &(channels[note->channel]);
            if (note->changed || channel->changed)
            {
                note->changed = false;

                if (note->number <= 127 && note->velocity)
                {
                    float noteNumFloat =   float(note->number)
                                         + channel->pitchBend * channel->pitchBendRange
                                         + channel->tuning;
                    note->frequency = exp2f((noteNumFloat - 69.0f) / 12.0f) * 440.0f;

                    // Determine MIDI volume, including all effects that are not time-dependant.
                    note->rawVolume = note->velocity / 128.0f;
                    if (channel->damperPedal)
                    {
                        note->rawVolume *= 0.6f;
                    }

                    float oldPan = note->pan;
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
                            note->pan = channel->notePanTargetRangeLow;
                        }
                        else if (noteNumFloat >= channel->notePanSourceRangeHigh)
                        {
                            note->pan = channel->notePanTargetRangeHigh;
                        }
                        else
                        {
                            note->pan =   (noteNumFloat - channel->notePanSourceRangeLow)
                                        * (channel->notePanTargetRangeHigh - channel->notePanTargetRangeLow)
                                        / (channel->notePanSourceRangeHigh - channel->notePanSourceRangeLow)
                                        +  channel->notePanTargetRangeLow;
                        }
                    }
                    else
                    {
                        note->pan = channel->pan;
                    }
                    if (note->pan != oldPan)
                    {
                        note->panChanged = COIL_COUNT;
                    }

                    note->rawVolume *= channel->volume * channel->expression;
                }
                else if (!channel->sustainPedal)
                {
                    note->ADSRStep = MIDIProgram::DATA_POINTS - 1;
                }
            }
            updateEffects(note);
        }
        for (uint32_t channel = 0; channel < 16; channel++)
        {
            channels[channel].changed = false;
        }
        removeDeadNotes();
    }
}

void MIDI::setPanVol(Note* note)
{
    // 1.01f instead of 1.0f to include the borders of the range.
    if (note->panChanged || coilPanChanged)
    {
        if (note->panChanged)
        {
            note->panChanged--;
        }
        note->panVol[coilNum] = 1.01f - inversPanReach * fabsf(note->pan - coilPan);
        if (note->panVol[coilNum] <= 0.0f)
        {
            note->panVol[coilNum] = 0.0f;
        }
        else if (panConstVol)
        {
            note->panVol[coilNum] = 1.0f;
        }

        if (coilPan < 0.0f || channels[note->channel].notePanOmniMode)
        {
            note->panVol[coilNum] = 1.0f;
        }
    }
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
        return (sinf(6.283185307179586f * float(System::getSystemTimeUS()) / LFO_PERIOD_US) + 1) / 4.0f
                * channels[channel].modulation;
    }
    else
    {
        return 0.0f;
    }
}

void MIDI::updateEffects(Note* note)
{
    // Needed for ADSR
    float currentTime = System::getSystemTimeUS();
    if (note->ADSRTimeUS == 0.0f)
    {
        note->ADSRTimeUS = currentTime;
    }
    float timeDiffUS = currentTime - note->ADSRTimeUS;
    if (timeDiffUS > effectResolutionUS)
    {
        note->ADSRTimeUS = currentTime;
        //for (uint32_t note = 0; note < MAX_VOICES; note++)
//        {
            /*if (note >= notesCount)
            {
                break;
            }*/
            uint32_t program = channels[note->channel].program;
            if (program)
            {
                float targetAmp = programs[program].amplitude[note->ADSRStep];
                float coefficient = programs[program].coefficient[note->ADSRStep];

                note->ADSRVolume += coefficient * timeDiffUS;

                if (    (note->ADSRVolume >= targetAmp && coefficient >= 0)
                     || (note->ADSRVolume <= targetAmp && coefficient <= 0))
                {
                    note->ADSRVolume = targetAmp;
                    if (note->ADSRStep < MIDIProgram::DATA_POINTS - 2)
                    {
                        note->ADSRStep++;
                    }
                }
            }
            else
            {
                // No ADSR calculations. Last data point is off, others are on.
                if (note->ADSRStep < MIDIProgram::DATA_POINTS - 1)
                {
                    // ADSRStep must not be 0 otherwise it will not be removed by MIDI::removeDeadNotes
                    note->ADSRStep = 1;

                    note->ADSRVolume = 1.0f;
                }
                else
                {
                    note->ADSRVolume = 0.0f;
                }
            }

            // After calculation of ADSR envelope, add other effects like modulation
            note->finishedVolume =   note->rawVolume
                                          * note->ADSRVolume
                                          * (1.0f - getLFOVal(note->channel));
//        }
    }
    else if (timeDiffUS < 0.0f)
    {
        // Time overflowed
        note->ADSRTimeUS = currentTime;
    }

}

void MIDI::removeDeadNotes()
{
    uint32_t deadNotes = 0;
    for (uint32_t note = 0; note < MAX_NOTES_COUNT; note++)
    {
        if (note >= notesCount)
        {
            break;
        }
        if (notes[note]->number >= 128)
        {
            deadNotes++;
            notes[note]->number = 0;
            for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
            {
                notes[note]->assignedTones[coil] = 0;
            }
        }
        else if (deadNotes)
        {
            Note *tempNote          = notes[note - deadNotes];
            notes[note - deadNotes] = notes[note];
            notes[note]             = tempNote;
        }
    }
    notesCount -= deadNotes;
}

void MIDI::updateToneList()
{
    Tone* lastTone = (Tone*) 1;
    for (uint32_t i = 0; i < MAX_NOTES_COUNT; i++)
    {
        if (i >= notesCount)
        {
            break;
        }
        Note* note = notes[i];
        if (channels[note->channel].coils & coilBit)
        {
            if (note->isDead())
            {
                note->number = 128; // Mark for removal.
                tonelist->deleteTone(note->assignedTones[coilNum]);
                note->assignedTones[coilNum] = 0;
            }
            else
            {
                setPanVol(note);
                if (lastTone/* && voicesLeft--*/)
                {
                    float periodUS = 1e6f / note->frequency;
                    float ontimeUS = note->finishedVolume * note->panVol[coilNum];
                    if (note->frequency >= absFreq)
                    {
                        ontimeUS *= singleNoteMaxOntimeUS;
                    }
                    else
                    {
                        ontimeUS *= singleNoteMaxDuty * periodUS;
                    }
                    lastTone = tonelist->updateTone(ontimeUS, periodUS, this, note, note->assignedTones[coilNum]);
                    note->assignedTones[coilNum] = lastTone;
                }
            }
        }
    }
    coilPanChanged = false;
}

void MIDI::setCoilNum(uint32_t num)
{
    coilNum = num;
    coilBit = 1 << num;
}
