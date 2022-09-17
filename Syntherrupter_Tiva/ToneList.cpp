/*
 * ToneList.cpp
 *
 *  Created on: 17.08.2020
 *      Author: Max Zuidberg
 */

#include <ToneList.h>

ToneList::ToneList()
{
    // For debugging purposes only.
    for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
    {
        unorderedTones[tone].id = tone;
        unorderedTones[tone].setParentList(this);
    }
    buildLinks();
}

ToneList::~ToneList()
{
    // Auto-generated destructor stub
}

Tone* ToneList::updateTone(uint32_t ontimeUS, uint32_t periodUS, void* owner, void* origin, Tone* tone)
{
    Tone* targetTone = 0;
    if (tone)
    {
        if (tone->owner == owner)
        {
            targetTone = tone;
        }
    }
    if (!targetTone)
    {
        targetTone = newTone;
        newTone    = newTone->nextTone;
        if (++activeTones > maxVoices)
        {
            deleteTone(firstTone);
        }
        targetTone->type  = Tone::Type::newdflt;
        targetTone->owner = owner;
    }

    bool newDuty = false;
    if (periodUS != targetTone->periodUS)
    {
        newDuty = true;
        targetTone->nextFireUS -= targetTone->periodUS;
        targetTone->nextFireUS += periodUS;
        targetTone->periodUS = periodUS;
    }
    ontimeUS = Branchless::min(ontimeUS, (uint32_t)maxOntimeUS);
    if (ontimeUS != targetTone->ontimeUS)
    {
        newDuty = true;
        targetTone->ontimeUS = ontimeUS;
        targetTone->limitedOntimeUS = ontimeUS;
    }
    if (newDuty)
    {
        targetTone->duty = float(ontimeUS) / float(periodUS);
        limit();
    }
    targetTone->origin = origin;
    return targetTone;
}

void ToneList::deleteTone(Tone* tone)
{
    if (tone)
    {
        activeTones--;

        tone->ontimeUS = 0;
        tone->owner    = 0;

        if (tone == firstTone)
        {
            firstTone = firstTone->nextTone;
        }
        else if (tone->nextTone == firstTone)
        {
            newTone = tone;
        }
        else
        {
            tone->prevTone->nextTone = tone->nextTone;
            tone->nextTone->prevTone = tone->prevTone;
            tone->nextTone           = firstTone;
            tone->prevTone           = firstTone->prevTone;
            tone->prevTone->nextTone = tone;
            tone->nextTone->prevTone = tone;
        }
    }
}

void ToneList::buildLinks()
{
    /*
     * This is a doubly linked list, but a circular one. This makes it easier
     * to handle "overflows" of the list but (seems) to require one additional
     * list element that won't be used/moved. Maybe a bug; haven't really
     * understood the issue yet. TODO!
     */
    for (uint32_t tone = 0; tone < MAX_VOICES - 1; tone++)
    {
        unorderedTones[tone].nextTone = &(unorderedTones[tone + 1]);
        unorderedTones[tone + 1].prevTone = &(unorderedTones[tone]);
    }
    firstTone = &(unorderedTones[0]);
    firstTone->prevTone = &(unorderedTones[MAX_VOICES - 1]);
    firstTone->prevTone->nextTone = firstTone;
    newTone  = firstTone;
}

void ToneList::limit()
{
    bool stillActive = false;
    float totalDuty = 0.0f;
    Tone* tone = firstTone;
    for (uint32_t toneNum = 0; toneNum < MAX_VOICES; toneNum++)
    {
        if (tone == newTone)
        {
            break;
        }
        totalDuty += tone->duty;
        tone = tone->nextTone;
    }
    signalDuty = totalDuty;
    if (totalDuty > maxDuty)
    {
        // Duty of all notes together exceeds coil limit; reduce ontimes.

        // Factor by which ontimes must be reduced.
        totalDuty = maxDuty / totalDuty;
        limiterActive = true;
        stillActive = true;
    }
    else
    {
        totalDuty = 1.0f;
    }
    if (limiterActive)
    {
        tone = firstTone;
        for (uint32_t toneNum = 0; toneNum < MAX_VOICES; toneNum++)
        {
            if (tone == newTone)
            {
                break;
            }
            tone->limitedOntimeUS = tone->ontimeUS * totalDuty;
            tone = tone->nextTone;
        }
    }
    limiterActive = stillActive;
}

void ToneList::applyTimeOffsetUS(uint32_t offsetUS)
{
    Tone* tone = firstTone;
    while (tone != newTone)
    {
        if (tone->nextFireUS > offsetUS)
        {
            tone->nextFireUS -= offsetUS;
        }
        tone = tone->nextTone;
    }
}
