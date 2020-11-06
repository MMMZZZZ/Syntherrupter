/*
 * Channel.cpp
 *
 *  Created on: 10.05.2020
 *      Author: Max Zuidberg
 */

#include <Channel.h>

Channel::Channel()
{
    resetControllers();
    program = 0;
    coils   = 0xff; // Startup default: play every channel on every coil.
}

Channel::~Channel()
{
    // TODO Auto-generated destructor stub
}

void Channel::resetControllers()
{
    channelAfterTouch            = 0;
    volume                       = 1.0f;
    expression                   = 1.0f;
    pitchBend                    = 0.0f;
    modulation                   = 0.0f;
    pan                          = 0.5f;
    tuning                       = 0.0f;
    pitchBendRange               = 2.0f / 8192.0f; // Unit: Semitones / 8192 - since the pitchbend value ranges from -8192 to 8192. Taking that factor in the range reduces the amount of calcs.
    sustainPedal                 = false;
    damperPedal                  = false;
    pitchBendRangeFine           = 0;
    pitchBendRangeCoarse         = 0;
    fineTuningFine               = 0;
    fineTuningCoarse             = 0;
    coarseTuning                 = 0;
    RPN                          = 0x7f7f;
    NRPN                         = 0x7f7f;
    controllersChanged           = true;
}

void Channel::resetNRPs()
{
    notePanSourceRangeLow  = 0.0f;
    notePanSourceRangeHigh = 1.0f;
    notePanTargetRangeLow  = 0.0f;
    notePanTargetRangeHigh = 1.0f;
    notePanMode            = NOTE_PAN_OFF;
    controllersChanged     = true;
}

Note* Channel::getNote(uint8_t noteNum)
{
    Note* note = firstNote;

    while (note != 0)
    {
        if(noteNum == note->number)
        {
            return note;
        }
        note = note->nextChnNote;
    }
    return 0;
}

void Channel::addNote(Note* note)
{
    note->nextChnNote = firstNote;
    firstNote = note;
    noteCount++;
}

void Channel::removeNote(Note* note)
{
    if (note == firstNote)
    {
        firstNote = firstNote->nextChnNote;
    }
    else
    {
        Note* prevNote = firstNote;
        Note* currentNote = firstNote->nextChnNote;
        while (currentNote != 0)
        {
            if (currentNote == note)
            {
                prevNote->nextChnNote = currentNote->nextChnNote;
                break;
            }
            prevNote    = currentNote;
            currentNote = currentNote->nextChnNote;
        }
    }

    note->nextChnNote = 0;
    noteCount--;
}

void Channel::notePanDataUpdate()
{
    Note* note = firstNote;

    float temp = 0.0f;

    switch (notePanMode)
    {
        case NOTE_PAN_LOWEST:
        {
            temp = 265.0f;
            while (note != 0)
            {
                if (note->pitch < temp)
                {
                    temp = note->pitch;
                    notePan = note->pitch;
                }
                note = note->nextChnNote;
            }
            break;
        }
        case NOTE_PAN_HIGHEST:
        {
            temp = 0.0f;
            while (note != 0)
            {
                if (note->pitch > temp)
                {
                    temp = note->pitch;
                    notePan = note->pitch;
                }
                note = note->nextChnNote;
            }
            break;
        }
        case NOTE_PAN_AVG:
        {
            temp = 0.0f;
            while (note != 0)
            {
                temp += note->pitch;
                note = note->nextChnNote;
            }
            temp /= noteCount;
            notePan = note->pitch;
            break;
        }
        case NOTE_PAN_LOUDEST:
        {
            temp = 0.0f;
            while (note != 0)
            {
                if (note->velocity > temp)
                {
                    temp = note->velocity;
                    notePan = note->pitch;
                }
                note = note->nextChnNote;
            }
            break;
        }
    }
}
