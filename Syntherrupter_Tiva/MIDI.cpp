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
Channel               MIDI::channels[];
NoteList              MIDI::notelist;
uint32_t              MIDI::notesCount = 0;
bool                  MIDI::playing = false;
float                 MIDI::freqTable[128];
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

    MIDIProgram::setResolutionUS(effectResolutionUS);

    // Copy legacy ADSR table to new MIDIProgram class. That's the way it works until there's a useful editor for ADSR.
    for (uint32_t prog = 0; prog < ADSR_PROGRAM_COUNT + 1; prog++)
    {
        programs[prog].setMode(MIDIProgram::Mode::lin);
        programs[prog+10].setMode(MIDIProgram::Mode::exp);
        for (uint32_t i = 0; i < 4; i++)
        {
            uint32_t dataPnt = i;
            uint32_t nextPnt = dataPnt + 1;
            if (dataPnt == 2)
            {
                nextPnt = dataPnt;
            }
            if (dataPnt == 3)
            {
                dataPnt = MIDIProgram::DATA_POINTS - 1;
                nextPnt = dataPnt;
            }
            programs[prog   ].setDataPoint(dataPnt, ADSR_LEGACY_PROGRAMS[prog][i*2], 1.0f / ADSR_LEGACY_PROGRAMS[prog][i*2+1], 0.0f, nextPnt);
            programs[prog+10].setDataPoint(dataPnt, ADSR_LEGACY_PROGRAMS[prog][i*2], 1.0f / ADSR_LEGACY_PROGRAMS[prog][i*2+1], 3.0f, nextPnt);
        }
    }

    for (uint32_t note = 0; note < 128; note++)
    {
        freqTable[note] = exp2f((note - 69.0f) / 12.0f) * 440.0f;
    }
}

bool MIDI::processBuffer(uint32_t b)
{
    /*
     * Minimum MIDI data processing.
     * Returns if valid data has been found.
     */

    /*
     * It would probably be better to have a child class of the ByteBuffer
     * class which does the buffer processing. But for now this does the
     * job, too.
     * It makes sure that the processing of the buffers is not mixed together.
     *
     */
    static uint32_t midiStatusAll[BUFFER_COUNT]{0};
    static uint32_t dataBytesAll[BUFFER_COUNT]{0};
    static uint32_t channelAll[BUFFER_COUNT]{0};
    static uint8_t  c1All[BUFFER_COUNT]{0};

    uint32_t* midiStatus = &(midiStatusAll[b]);
    uint32_t* dataBytes  = &(dataBytesAll[b]);
    uint32_t* channel    = &(channelAll[b]);
    uint8_t*  c1         = &(c1All[b]);

    ByteBuffer* buffer = BUFFER_LIST[b];

    *c1 = buffer->read();

    if (*c1 & 0b10000000) // The first byte of a MIDI Command starts with a 1. Following bytes start with a 0.
    {
        if (0xf8 <= *c1)
        {
            // System real time messages do not affect the running status.
        }
        else
        {
            // Lower 4 bits are channel.
            *channel = *c1 & 0x0f;

            // Save running status
            *midiStatus = *c1;
        }
        *dataBytes = 0;
    }
    else
    {
        (*dataBytes)++;

    }

    switch ((*midiStatus) & 0xf0)
    {
        case 0x80: // Note off
        {
            if (*dataBytes == 1)
            {
                Note* note = channels[*channel].getNote(*c1);
                if (note)
                {
                    note->velocity = 0;
                    note->changed  = true;
                    channels[*channel].notesChanged = true;
                }
            }
            break;
        }
        case 0x90: // Note on
        {
            static Note*    noteAll[BUFFER_COUNT]{0};
            static uint8_t  numberAll[BUFFER_COUNT]{0};
            Note**   note   = &(noteAll[b]);
            uint8_t* number = &(numberAll[b]);
            if (*dataBytes == 1)
            {
                *number = *c1;
                *note = channels[*channel].getNote(*c1);
            }
            else if (*dataBytes == 2)
            {
                if (*c1) // Note has a velocity
                {
                    if (!(*note))
                    {
                        (*note) = notelist.addNote();
                        (*note)->afterTouch  = 0;
                        (*note)->rawVolume   = 0.0f;
                        (*note)->ADSRVolume  = 0.0f;
                        channels[*channel].addNote(*note);
                    }

                    (*note)->number     = *number;
                    (*note)->velocity   = *c1;
                    (*note)->ADSRStep   = 0;
                    (*note)->ADSRTimeUS = 0.0f;
                    (*note)->channel    = *channel;
                    (*note)->changed    = true;
                    channels[*channel].notesChanged = true;
                }
                else if (*note) // Note has no velocity = note off. Code copy pasted from note off command.
                {
                    (*note)->velocity = 0;
                    (*note)->changed  = true;
                    channels[*channel].notesChanged = true;
                }
            }
            break;
        }
        case 0xA0: // Polyphonic Aftertouch
        {
            static Note* noteAll[BUFFER_COUNT]{0};
            Note** note = &(noteAll[b]);
            if (*dataBytes == 1)
            {
                (*note) = notelist.getNote(*channel, *c1);
            }
            else if (*dataBytes == 2 && (*note))
            {
                (*note)->afterTouch = *c1;
                (*note)->changed = true;
                channels[*channel].notesChanged = true;
            }
            break;
        }
        case 0xB0: // Control Change / Channel Mode
        {
            static uint32_t controllerAll[BUFFER_COUNT]{128};
            uint32_t* controller = &(controllerAll[b]);
            if (*dataBytes == 1)
            {
                *controller = *c1;
            }
            else if (*dataBytes == 2)
            {
                switch (*controller)
                {
                    default:
                        break;
                    case 0x01: // Modulation Wheel
                        channels[*channel].modulation = *c1 / 128.0f;
                        channels[*channel].controllersChanged = true;
                        break;
                    case 0x02: // Breath Controller

                        break;
                    case 0x07: // Channel Volume
                        channels[*channel].volume = *c1 / 128.0f;
                        channels[*channel].controllersChanged = true;
                        break;
                    case 0x0A: // Pan
                        channels[*channel].pan = *c1 / 128.0f;
                        channels[*channel].controllersChanged = true;
                        break;
                    case 0x0B: // Expression coarse
                        channels[*channel].expression = *c1 / 128.0f;
                        channels[*channel].controllersChanged = true;
                        break;
                    case 0x40: // Sustain Pedal
                        if (*c1 >= 64)
                        {
                            channels[*channel].sustainPedal = true;
                        }
                        else
                        {
                            channels[*channel].sustainPedal = false;
                        }
                        channels[*channel].controllersChanged = true;
                        break;
                    case 0x43: // Damper Pedal
                        if (*c1 >= 64)
                        {
                            channels[*channel].damperPedal = true;
                        }
                        else
                        {
                            channels[*channel].damperPedal = false;
                        }
                        channels[*channel].controllersChanged = true;
                        break;
                    case 0x62: // Non Registered Parameter Number, fine
                        channels[*channel].NRPN &= 0xff00;
                        channels[*channel].NRPN += *c1;

                        // Can't receive RP and NRP data at the same time.
                        channels[*channel].RPN   = 0x7f7f;
                        break;
                    case 0x63: // Non Registered Parameter Number, coarse
                        channels[*channel].NRPN &= 0x00ff;
                        channels[*channel].NRPN += (*c1 << 8);

                        // Can't receive RP and NRP data at the same time.
                        channels[*channel].RPN   = 0x7f7f;
                        break;
                    case 0x64: // Registered Parameter Number, fine
                        channels[*channel].RPN &= 0xff00;
                        channels[*channel].RPN += *c1;

                        // Can't receive RP and NRP data at the same time.
                        channels[*channel].NRPN = 0x7f7f;
                        break;
                    case 0x65: // Registered Parameter Number, coarse
                        channels[*channel].RPN &= 0x00ff;
                        channels[*channel].RPN += (*c1 << 8);

                        // Can't receive RP and NRP data at the same time.
                        channels[*channel].NRPN = 0x7f7f;
                        break;
                    case 0x06: // (N)RPN Data Entry, coase
                    {
                        bool previousState = channels[*channel].controllersChanged;
                        channels[*channel].controllersChanged = true;

                        // Registered Parameter
                        if (channels[*channel].RPN == 0) // Pitch bend range
                        {
                            channels[*channel].pitchBendRangeCoarse = *c1;
                            channels[*channel].pitchBendRange  =   channels[*channel].pitchBendRangeCoarse
                                                                 + channels[*channel].pitchBendRangeFine / 100.0f;
                            channels[*channel].pitchBendRange /= 8192.0f;
                        }
                        else if (channels[*channel].RPN == 1) // Fine tuning
                        {
                            channels[*channel].fineTuningCoarse = *c1;
                            channels[*channel].tuning = ((channels[*channel].fineTuningCoarse << 8)
                                                        + channels[*channel].fineTuningFine) - 8192.0f;
                            channels[*channel].tuning /= 4096.0f;
                            channels[*channel].tuning += channels[*channel].coarseTuning;
                        }
                        else if (channels[*channel].RPN == 2) // Coarse tuning
                        {
                            channels[*channel].coarseTuning = *c1;
                            channels[*channel].tuning = ((channels[*channel].fineTuningCoarse << 8)
                                                        + channels[*channel].fineTuningFine) - 8192.0f;
                            channels[*channel].tuning /= 4096.0f;
                            channels[*channel].tuning += channels[*channel].coarseTuning;
                        }
                        // Non-Registered Parameter
                        else if (channels[*channel].NRPN == (42 << 8) + 1) // Note pan mode - source range upper limit
                        {
                            channels[*channel].notePanSourceRangeHigh = *c1;
                        }
                        else if (channels[*channel].NRPN == (42 << 8) + 2) // Note pan mode - target range upper limit
                        {
                            channels[*channel].notePanTargetRangeHigh = *c1 / 128.0f;
                        }
                        else
                        {
                            channels[*channel].controllersChanged = previousState;
                        }
                        break;
                    }
                    case 0x26: // (N)RPN Data Entry, fine
                    {
                        bool previousState = channels[*channel].controllersChanged;
                        channels[*channel].controllersChanged = true;

                        if (channels[*channel].RPN == 0) // Pitch bend range
                        {
                            channels[*channel].pitchBendRangeFine = *c1;
                            channels[*channel].pitchBendRange  =   channels[*channel].pitchBendRangeCoarse
                                                                 + channels[*channel].pitchBendRangeFine / 100.0f;
                            channels[*channel].pitchBendRange /= 8192.0f;
                        }
                        else if (channels[*channel].RPN == 1) // Fine tuning
                        {
                            /*
                             * Fine tuning mapping is similar to pitch bend. A 14 bit value (0..16383) is mapped to -2.0f..2.0f
                             * Coarse tuning is unmapped.
                             */
                            channels[*channel].fineTuningFine = *c1;
                            channels[*channel].tuning = ((channels[*channel].fineTuningCoarse << 8)
                                                        + channels[*channel].fineTuningFine) - 8192.0f;
                            channels[*channel].tuning /= 4096.0f;
                            channels[*channel].tuning += channels[*channel].coarseTuning;
                        }
                        // Non-Registered Parameter
                        else if (channels[*channel].NRPN == (42 << 8) + 0) // Note pan mode - enable/disable
                        {
                            channels[*channel].notePanMode = *c1;
                            /*if (*c1 == 2)
                            {
                                // Omni Mode (Note plays everywhere)
                                channels[*channel].notePanOmniMode = true;
                            }
                            else
                            {
                                channels[*channel].notePanOmniMode = false;
                                channels[*channel].notePanEnabled = *c1;
                            }*/
                        }
                        else if (channels[*channel].NRPN == (42 << 8) + 1) // Note pan mode - source range lower limit
                        {
                            channels[*channel].notePanSourceRangeLow = *c1;
                        }
                        else if (channels[*channel].NRPN == (42 << 8) + 2) // Note pan mode - target range lower limit
                        {
                            channels[*channel].notePanTargetRangeLow = *c1 / 128.0f;
                        }
                        else
                        {
                            channels[*channel].controllersChanged = previousState;
                        }
                        break;
                    }
                    case 0x78: // All Sounds off
                    {
                        Note* note = channels[*channel].firstNote;
                        while (note != 0)
                        {
                            Note* nextNote = note->nextChnNote;
                            channels[*channel].removeNote(note);
                            notelist.removeNote(note);
                            note = nextNote;
                        }
                        break;
                    }
                    case 0x79: // Reset all Controllers
                        channels[*channel].resetControllers();
                        break;
                    case 0x7B: // All Notes off
                    {
                        Note* note = channels[*channel].firstNote;
                        while (note != 0)
                        {
                            note->ADSRStep = MIDIProgram::DATA_POINTS - 1;
                            note = note->nextChnNote;
                        }
                        break;
                    }
                }
            }
            break;
        }
        case 0xE0: // Pitch Bend
        {
            static float pb{0.0f};
            if (*dataBytes == 1)
            {
                pb = *c1 - 8192.0f;
            }
            else if (*dataBytes == 2)
            {
                channels[*channel].pitchBend = *c1 * 128.0f + pb;
                channels[*channel].controllersChanged = true;
            }
            break;
        }
        case 0xD0: // Channel Aftertouch
        {
            if (*dataBytes == 1)
            {
                channels[*channel].channelAfterTouch = *c1;
                channels[*channel].controllersChanged = true;
            }
            break;
        }
        case 0xC0: // Program Change
        {
            if (*dataBytes == 1)
            {
                if (*c1 <= MAX_PROGRAMS)
                {
                    channels[*channel].program = *c1;
                }
                else
                {
                    channels[*channel].program = 0;
                }
            }
            break;
        }
    }

    return true;
}

void MIDI::start()
{
    if (!playing)
    {
        playing = true;
        usbUart.enable();
        midiUart.enable();
        for (uint32_t channel = 0; channel < 16; channel++)
        {
            channels[channel].resetControllers();
        }
    }
}

void MIDI::stop()
{
    playing = false;
    usbUart.disable();
    midiUart.disable();
    notelist.removeAllNotes();
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

    coilChange = true;
}

void MIDI::setChannels(uint32_t chns)
{
    activeChannels = chns;
    for (uint32_t i = 0; i < 16; i++)
    {
        if (chns & (1 << i))
        {
            channels[i].coils |=  coilBit;
        }
        else
        {
            channels[i].coils &= ~coilBit;
        }
    }

    coilChange = true;
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
    coilChange = true;
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
            channels[i].controllersChanged = true;
        }
    }
}

void MIDI::process()
{
    if (playing)
    {
        // Process all data that's in the buffer.

        bool newData = false;
        for (uint32_t bufferNum = 0; bufferNum < BUFFER_COUNT; bufferNum++)
        {
            while (BUFFER_LIST[bufferNum]->level())
            {
                newData = true;
                processBuffer(bufferNum);
            }
        }

        if (newData)
        {
            for (uint32_t channelNum = 0; channelNum < 16; channelNum++)
            {
                Channel* channel = &(channels[channelNum]);

                if (channel->notesChanged || channel->controllersChanged)
                {
                    Note* note = channel->firstNote;
                    while (note != 0)
                    {
                        if (channel->controllersChanged || note->changed)
                        {
                            note->changed = false;
                            note->toneChanged = (1 << COIL_COUNT) - 1;

                            if (note->number <= 127 && note->velocity)
                            {
                                note->pitch =   float(note->number)
                                              + channel->pitchBend * channel->pitchBendRange
                                              + channel->tuning;
                                note->frequency = getFreq(note->pitch);
                                note->periodUS = 1e6f / note->frequency;

                                // Determine MIDI volume, including all effects that are not time-dependant.
                                note->rawVolume = note->velocity / 128.0f;
                                if (channel->damperPedal)
                                {
                                    note->rawVolume *= 0.6f;
                                }
                                note->rawVolume *= channel->volume * channel->expression;
                            }
                            else if (!channel->sustainPedal)
                            {
                                note->ADSRStep = MIDIProgram::DATA_POINTS - 1;
                            }
                        }
                        note = note->nextChnNote;
                    }

                    channel->notePanDataUpdate();
                    note = channel->firstNote;
                    while (note != 0)
                    {
                        float oldPan = note->pan;
                        if (channel->notePanMode != Channel::NOTE_PAN_OFF)
                        {
                            /*
                             * In this mode a note number determines the pan position of the note.
                             * This is done with a mapping. A source range, covering any part of
                             * the [0..127] note number range, will be mapped to a target range,
                             * covering any part of the [0..1] pan position range.
                             */
                            float notePanInput = 0.0f;
                            if (channel->notePanMode == Channel::NOTE_PAN_INDIVIDUAL)
                            {
                                notePanInput = note->pitch;
                            }
                            else
                            {
                                notePanInput = channel->notePan;
                            }
                            if (notePanInput <= channel->notePanSourceRangeLow)
                            {
                                note->pan = channel->notePanTargetRangeLow;
                            }
                            else if (notePanInput >= channel->notePanSourceRangeHigh)
                            {
                                note->pan = channel->notePanTargetRangeHigh;
                            }
                            else
                            {
                                note->pan =   (notePanInput - channel->notePanSourceRangeLow)
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
                            note->panChanged = (1 << COIL_COUNT) - 1;
                        }

                        note = note->nextChnNote;
                    }
                    channel->controllersChanged = false;
                    channel->notesChanged       = false;
                }
            }
        }

        Note* note = notelist.firstNote;
        while (note != notelist.newNote)
        {
            updateEffects(note);
            note = note->nextNote;
        }
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
    else
    {
        float timeDiffUS = currentTime - note->ADSRTimeUS;
        if (timeDiffUS > effectResolutionUS)
        {
            note->ADSRTimeUS = currentTime;
            MIDIProgram* program = &(programs[channels[note->channel].program]);
            if (!program->setADSRAmp(&(note->ADSRStep), &(note->ADSRVolume)))
            {
                // Note ended.
                note->number = 128;
                note->toneChanged = (1 << COIL_COUNT) - 1;
            }
            else
            {

                // After calculation of ADSR envelope, add other effects like modulation
                float finishedVolume =   note->rawVolume
                                       * note->ADSRVolume
                                       * (1.0f - getLFOVal(note->channel));
                if (finishedVolume != note->finishedVolume)
                {
                    note->toneChanged = (1 << COIL_COUNT) - 1;
                    note->finishedVolume = finishedVolume;
                }
            }
        }
        if (timeDiffUS < 0.0f)
        {
            // Time overflowed
            note->ADSRTimeUS = currentTime;
        }
    }
}

void MIDI::updateToneList()
{
    Tone* lastTone = (Tone*) 1;
    Note* note = notelist.firstNote;
    uint32_t voicesLeft = coilMaxVoices;
    while (note != notelist.newNote)
    {
        Note* nextNote = note->nextNote;
        if (note->toneChanged & coilBit || coilChange)
        {
            note->toneChanged &= ~coilBit;

            Tone** assignedTone = &(note->assignedTones[coilNum]);

            if (note->isDead())
            {
                channels[note->channel].removeNote(note);
                notelist.removeNote(note); // Removes tone from tonelists, too.
            }
            else if (channels[note->channel].coils & coilBit)
            {
                setPanVol(note);
                if (lastTone && voicesLeft)
                {
                    float ontimeUS = note->finishedVolume * note->panVol[coilNum];
                    if (note->frequency >= absFreq)
                    {
                        ontimeUS *= singleNoteMaxOntimeUS;
                    }
                    else
                    {
                        ontimeUS *= singleNoteMaxDuty * note->periodUS;
                    }
                    if (*assignedTone)
                    {
                        if ((*assignedTone)->owner != this)
                        {
                            *assignedTone = 0;
                        }
                    }
                    if (ontimeUS < 1.0f)
                    {
                        if (*assignedTone)
                        {
                            (*assignedTone)->remove(note);
                            *assignedTone = 0;
                        }
                    }
                    else
                    {
                        voicesLeft--;
                        lastTone = tonelist->updateTone(ontimeUS, note->periodUS, this, note, *assignedTone);
                        *assignedTone = lastTone;
                    }
                }
            }
            else
            {
                // This coil is no more listening to this channel. Remove the
                // assigned tone if there is one.
                if (*assignedTone)
                {
                    (*assignedTone)->remove(note);
                     *assignedTone = 0;
                }
            }
        }

        note = nextNote;
    }
    coilChange     = false;
    coilPanChanged = false;
}

void MIDI::setCoilNum(uint32_t num)
{
    coilNum = num;
    coilBit = 1 << num;
}
