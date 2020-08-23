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
constexpr ByteBuffer* MIDI::bufferList[];
uint8_t               MIDI::bufferMidiStatus[] = {0, 0, 0};
Channel               MIDI::channels[];
Note                  MIDI::notes[];
Note*                 MIDI::orderedNotes[];
uint32_t              MIDI::notesCount         = 0;
bool                  MIDI::playing            = false;
constexpr float       MIDI::ADSR_PROGRAMS[ADSR_PROGRAM_COUNT + 1][9];
float                 MIDI::ADSRTimeUS;


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
    otherBuffer.init(128); // default buffer size, used in UART class, too.

    // Initialize static class variables
    for (uint32_t note = 0; note < totalNotesLimit; note++)
    {
        orderedNotes[note] = &(notes[note]);
    }
}

void MIDI::setCoilsToneList(ToneList* tonelist)
{
    this->tonelist = tonelist;
}

bool MIDI::processBuffer(uint32_t b)
{
    /*
     * Minimum MIDI data processing.
     * Returns if valid data has been found.
     */

    ByteBuffer* buffer = bufferList[b];
    uint32_t bufferLevel = buffer->level();

    if (!bufferLevel)
    {
        return false;
    }

    uint32_t channel = 0;

    uint8_t c1 = buffer->peek();

    if (c1 & 0b10000000) // The first byte of a MIDI Command starts with a 1. Following bytes start with a 0.
    {
        // Ignore System messages.
        if (0xf0 <= c1 && c1 <= 0xf7)
        {
            // System exclusive and system common messages reset the running status.
            bufferMidiStatus[b] = 0;
            return true;
        }
        else if (0xf8 <= c1)
        {
            // System real time messages do not affect the running status.
            return true;
        }
        else
        {
            bufferMidiStatus[b] = c1;
        }
    }
    else
    {
        // Lower 4 bits are channel.
        channel = bufferMidiStatus[b] & 0x0f;

        switch (bufferMidiStatus[b] & 0xf0)
        {
        case 0x80: // Note off
            if (bufferLevel >= 2)
            {
                for (uint32_t note = 0; note < totalNotesLimit; note++)
                {
                    if (note >= notesCount)
                    {
                        break;
                    }
                    if (c1 == notes[note].number && channel == notes[note].channel)
                    {
                        notes[note].velocity = 0;
                        notes[note].changed  = true;
                        break;
                    }
                }
                buffer->remove(2);
            }
            break;
        case 0x90: // Note on
            if (bufferLevel >= 2)
            {
                uint8_t c2 = buffer->peek(1);
                if (c2) // Note has a velocity
                {
                    if (channels[channel].coils)
                    {
                        uint32_t targetNote = notesCount;
                        bool foundNote = false;
                        for (uint32_t note = 0; note < totalNotesLimit; note++)
                        {
                            if (note >= notesCount)
                            {
                                break;
                            }
                            if (c1 == orderedNotes[note]->number
                                    && channel == orderedNotes[note]->channel)
                            {
                                targetNote = note;
                                foundNote = true;
                                break;
                            }
                        }
                        if (!foundNote)
                        {
                            if (targetNote == totalNotesLimit)
                            {
                                targetNote = totalNotesLimit - 1;
                                Note * tempNote = orderedNotes[0];
                                for (uint32_t note = 1; note < totalNotesLimit; note++)
                                {
                                    if (note >= totalNotesLimit)
                                    {
                                        break;
                                    }
                                    orderedNotes[note - 1] = orderedNotes[note];
                                }
                                // The other values will be overwritten anyway.
                                orderedNotes[targetNote] = tempNote;
                                orderedNotes[targetNote]->afterTouch = 0;
                                orderedNotes[targetNote]->rawVolume  = 0.0f;
                                orderedNotes[targetNote]->ADSRVolume = 0.0f;
                            }
                            else
                            {
                                notesCount++;
                            }
                        }

                        orderedNotes[targetNote]->number   = c1;
                        orderedNotes[targetNote]->velocity = c2;
                        orderedNotes[targetNote]->ADSRMode = 'A';
                        orderedNotes[targetNote]->channel  = channel;
                        orderedNotes[targetNote]->assignedTone = 0;
                        orderedNotes[targetNote]->changed  = true;
                    }
                }
                else // Note has no velocity = note off. Code copy pasted from note off command.
                {
                    for (uint32_t note = 0; note < totalNotesLimit; note++)
                    {
                        if (note >= notesCount)
                        {
                            break;
                        }
                        if (c1 == notes[note].number && channel == notes[note].channel)
                        {
                            notes[note].velocity = 0;
                            notes[note].changed  = true;
                            break;
                        }
                    }
                }
                buffer->remove(2);
            }
            break;
        case 0xA0: // Polyphonic Aftertouch
            if (bufferLevel >= 2)
            {
                uint8_t c2 = buffer->peek(1);
                if (channels[channel].coils)
                {
                    for (uint32_t note = 0; note < totalNotesLimit; note++)
                    {
                        if (c1 == notes[note].number && channel == notes[note].channel)
                        {
                            notes[note].afterTouch = c2;
                            notes[note].changed = true;
                            break;
                        }
                    }
                }
                buffer->remove(2);
            }
            break;
        case 0xB0: // Control Change / Channel Mode
            if (bufferLevel >= 2)
            {
                uint8_t c2 = buffer->peek(1);
                switch (c1)
                {
                case 0x01: // Modulation Wheel
                    channels[channel].modulation = c2 / 128.0f;
                    break;
                case 0x02: // Breath Controller

                    break;
                case 0x07: // Channel Volume
                    channels[channel].volume = c2 / 128.0f;
                    channels[channel].changed = true;
                    break;
                case 0x0A: // Pan
                    if (!channels[channel].notePanEnabled)
                    {
                        channels[channel].pan = c2 / 128.0f;
                        channels[channel].changed = true;
                    }
                    break;
                case 0x0B: // Expression coarse
                    channels[channel].expression = c2 / 128.0f;
                    break;
                case 0x40: // Sustain Pedal
                    if (c2 >= 64)
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
                    if (c2 >= 64)
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
                    channels[channel].NRPN += c2;

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].RPN   = 0x7f7f;
                    break;
                case 0x63: // Non Registered Parameter Number, coarse
                    channels[channel].NRPN &= 0x00ff;
                    channels[channel].NRPN += (c2 << 8);

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].RPN   = 0x7f7f;
                    break;
                case 0x64: // Registered Parameter Number, fine
                    channels[channel].RPN &= 0xff00;
                    channels[channel].RPN += c2;

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].NRPN = 0x7f7f;
                    break;
                case 0x65: // Registered Parameter Number, coarse
                    channels[channel].RPN &= 0x00ff;
                    channels[channel].RPN += (c2 << 8);

                    // Can't receive RP and NRP data at the same time.
                    channels[channel].NRPN = 0x7f7f;
                    break;
                case 0x06: // (N)RPN Data Entry, coase
                    // Registered Parameter
                    if (channels[channel].RPN == 0) // Pitch bend range
                    {
                        channels[channel].pitchBendRangeCoarse = c2;
                        channels[channel].pitchBendRange  =   channels[channel].pitchBendRangeCoarse
                                                                + channels[channel].pitchBendRangeFine / 100.0f;
                        channels[channel].pitchBendRange /= 8192.0f;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].RPN == 1) // Fine tuning
                    {
                        channels[channel].fineTuningCoarse = c2;
                        channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                        + channels[channel].fineTuningFine) - 8192.0f;
                        channels[channel].tuning /= 4096.0f;
                        channels[channel].tuning += channels[channel].coarseTuning;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].RPN == 2) // Coarse tuning
                    {
                        channels[channel].coarseTuning = c2;
                        channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                        + channels[channel].fineTuningFine) - 8192.0f;
                        channels[channel].tuning /= 4096.0f;
                        channels[channel].tuning += channels[channel].coarseTuning;
                        channels[channel].changed = true;
                    }

                    // Non-Registered Parameter
                    if (channels[channel].NRPN == (42 << 8) + 1) // Note pan mode - source range upper limit
                    {
                        channels[channel].notePanSourceRangeHigh = c2;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].NRPN == (42 << 8) + 2) // Note pan mode - target range upper limit
                    {
                        channels[channel].notePanTargetRangeHigh = c2 / 128.0f;
                        channels[channel].changed = true;
                    }
                    break;
                case 0x26: // (N)RPN Data Entry, fine
                    if (channels[channel].RPN == 0) // Pitch bend range
                    {
                        channels[channel].pitchBendRangeFine = c2;
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
                        channels[channel].fineTuningFine = c2;
                        channels[channel].tuning = ((channels[channel].fineTuningCoarse << 8)
                                                        + channels[channel].fineTuningFine) - 8192.0f;
                        channels[channel].tuning /= 4096.0f;
                        channels[channel].tuning += channels[channel].coarseTuning;
                        channels[channel].changed = true;
                    }

                    // Non-Registered Parameter
                    if (channels[channel].NRPN == (42 << 8) + 0) // Note pan mode - enable/disable
                    {
                        if (c2 == 2)
                        {
                            // Omni Mode (Note plays everywhere)
                            channels[channel].notePanOmniMode = true;
                        }
                        else
                        {
                            channels[channel].notePanOmniMode = false;
                            channels[channel].notePanEnabled = c2;
                        }
                    }
                    else if (channels[channel].NRPN == (42 << 8) + 1) // Note pan mode - source range lower limit
                    {
                        channels[channel].notePanSourceRangeLow = c2;
                        channels[channel].changed = true;
                    }
                    else if (channels[channel].NRPN == (42 << 8) + 2) // Note pan mode - target range lower limit
                    {
                        channels[channel].notePanTargetRangeLow = c2 / 128.0f;
                        channels[channel].changed = true;
                    }
                    break;
                case 0x78: // All Sounds off
                    if (channels[channel].coils)
                    {
                        for (uint32_t note = 0; note < totalNotesLimit; note++)
                        {
                            if (notes[note].channel == channel)
                            {
                                notes[note].number = 128;
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
                        for (uint32_t note = 0; note < totalNotesLimit; note++)
                        {
                            if (notes[note].channel == channel)
                            {
                                notes[note].ADSRMode = 'R';
                            }
                        }
                    }
                    break;
                }
                buffer->remove(2);
            }
            break;
        case 0xE0: // Pitch Bend
            if (bufferLevel >= 2)
            {
                uint8_t c2 = buffer->peek(1);
                channels[channel].pitchBend = ((c2 << 7) + c1);
                channels[channel].pitchBend -= 8192.0f;
                channels[channel].changed = true;
                buffer->remove(2);
            }
            break;
        case 0xD0: // Channel Aftertouch
            channels[channel].channelAfterTouch = c1;
            channels[channel].changed = true;
            buffer->remove(1);
            break;
        case 0xC0: // Program Change
            if (c1 <= ADSR_PROGRAM_COUNT)
            {
                channels[channel].program = c1;
            }
            else
            {
                channels[channel].program = 0;
            }
            buffer->remove(1);
            break;
        default:
            buffer->remove();
            break;
        }
    }

    return true;
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

void MIDI::process(bool force)
{
    // Process all data that's in the buffer.
    for (uint32_t i = 0; i < bufferCount; i++)
    {
        processBuffer(i);
    }

    for (uint32_t noteNum = 0; noteNum < totalNotesLimit; noteNum++)
    {
        if (noteNum >= notesCount)
        {
            break;
        }
        Note* note =  orderedNotes[noteNum];
        Channel* channel = &(channels[note->channel]);
        if (force || note->changed || channel->changed)
        {
            note->changed = false;

            if (noteNum <= 127 && note->velocity)
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
                note->ADSRMode = 'R';
            }
        }
    }
    for (uint32_t channel = 0; channel < 16; channel++)
    {
        channels[channel].changed = false;
    }

    updateEffects();
    removeDeadNotes();
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
    float timeDiffUS = currentTime - ADSRTimeUS;
    if (timeDiffUS > effectResolutionUS)
    {
        ADSRTimeUS = currentTime;
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            for (uint32_t note = 0; note < MAX_VOICES; note++)
            {
                if (note >= notesCount)
                {
                    break;
                }
                uint32_t program = channels[orderedNotes[note]->channel].program;
                Note* currentNote = orderedNotes[note];
                if (program)
                {
                    switch (currentNote->ADSRMode)
                    {
                    case 'A':
                        targetAmp         = ADSR_PROGRAMS[program][ADSR_ATTACK_AMP];
                        lastAmp           = ADSR_PROGRAMS[program][ADSR_RELEASE_AMP];
                        inversDurationUS  = ADSR_PROGRAMS[program][ADSR_ATTACK_INVDUR_US];
                        break;
                    case 'D':
                        targetAmp         = ADSR_PROGRAMS[program][ADSR_DECAY_AMP];
                        lastAmp           = ADSR_PROGRAMS[program][ADSR_ATTACK_AMP];
                        inversDurationUS  = ADSR_PROGRAMS[program][ADSR_DECAY_INVDUR_US];
                        break;
                    case 'S':
                        targetAmp         = ADSR_PROGRAMS[program][ADSR_SUSTAIN_AMP];
                        lastAmp           = ADSR_PROGRAMS[program][ADSR_DECAY_AMP];
                        inversDurationUS  = ADSR_PROGRAMS[program][ADSR_SUSTAIN_INVDUR_US];
                        break;
                    case 'R':
                        targetAmp         = ADSR_PROGRAMS[program][ADSR_RELEASE_AMP];
                        lastAmp           = ADSR_PROGRAMS[program][ADSR_SUSTAIN_AMP];
                        inversDurationUS  = ADSR_PROGRAMS[program][ADSR_RELEASE_INVDUR_US];
                        break;
                    }

                    ampDiff = targetAmp - lastAmp;

                    currentNote->ADSRVolume += ampDiff * timeDiffUS * inversDurationUS;
                    if (    (currentNote->ADSRVolume >= targetAmp && ampDiff >= 0)
                         || (currentNote->ADSRVolume <= targetAmp && ampDiff <= 0))
                    {
                        if (currentNote->ADSRMode == 'A')
                        {
                            currentNote->ADSRMode = 'D';
                        }
                        else if (currentNote->ADSRMode == 'D')
                        {
                            currentNote->ADSRMode = 'S';
                        }
                        currentNote->ADSRVolume = targetAmp;
                    }
                }
                else
                {
                    // No ADSR calculations. A/D/S = on, R = off
                    if (currentNote->ADSRMode != 'R')
                    {
                        // ADSRMode must not be 'A' otherwise it will not be removed by MIDI::removeDeadNotes
                        currentNote->ADSRMode = 'S';

                        currentNote->ADSRVolume = 1.0f;
                    }
                    else
                    {
                        currentNote->ADSRVolume = 0.0f;
                    }
                }

                // After calculation of ADSR envelope, add other effects like modulation
                currentNote->finishedVolume =   currentNote->rawVolume
                                              * currentNote->ADSRVolume
                                              * (1.0f - getLFOVal(currentNote->channel))
                                              * playing;
            }
        }
    }
    else if (timeDiffUS < 0.0f)
    {
        // Time overflowed
        ADSRTimeUS = currentTime;
    }

}

void MIDI::removeDeadNotes()
{
    uint32_t deadNotes = 0;
    for (uint32_t note = 0; note < totalNotesLimit; note++)
    {
        if (note >= notesCount)
        {
            break;
        }
        if (orderedNotes[note]->number >= 128)
        {
            deadNotes++;
            orderedNotes[note]->number       = 0;
            orderedNotes[note]->assignedTone = 0;
        }
        else if (deadNotes)
        {
            Note *tempNote                 = orderedNotes[note - deadNotes];
            orderedNotes[note - deadNotes] = orderedNotes[note];
            orderedNotes[note]             = tempNote;
        }
    }
    notesCount -= deadNotes;
}

void MIDI::updateToneList()
{
    Tone* lastTone = (Tone*) 1;
    uint32_t voicesLeft = coilMaxVoices;
    for (uint32_t i = 0; i < totalNotesLimit; i++)
    {
        if (i >= notesCount)
        {
            break;
        }
        Note* note = orderedNotes[i];
        if (channels[note->channel].coils & coilBit)
        {
            if (note->isDead())
            {
                note->number = 128; // Mark for removal.
                tonelist->deleteTone(note->assignedTone);
            }
            else
            {
                setPanVol(note);
                if (lastTone && voicesLeft--)
                {
                    float periodUS = 1.0f / note->frequency;
                    float ontimeUS = note->finishedVolume * note->panVol[coilNum];
                    if (note->frequency >= absFreq)
                    {
                        ontimeUS *= singleNoteMaxOntimeUS;
                    }
                    else
                    {
                        ontimeUS *= singleNoteMaxDuty * periodUS;
                    }
                    lastTone = tonelist->updateTone(ontimeUS, periodUS, this, note, note->assignedTone);
                    note->assignedTone = lastTone;
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
