/*
 * ToneList.cpp
 *
 *  Created on: 17.08.2020
 *      Author: Max Zuidberg
 */

#include <ToneList.h>

ToneList::ToneList()
{
    // To prevent excessive copy operations when ordering the tones,
    // tones only contains the pointers to the actual Tone objects.
    for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
    {
        unorderedTones[tone].id = tone;
    }
    buildLinks();
}

ToneList::~ToneList()
{
    // TODO Auto-generated destructor stub
}

/*void ToneList::removeDeadTones()
{
    uint32_t deadTones = 0;
    for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
    {
        if (tone >= activeTones)
        {
            break;
        }
        if (!tones[tone]->ontimeUS || !tones[tone]->nextFireUS)
        {
            deadTones++;
            tones[tone]->owner      = 0;
            tones[tone]->origin     = 0;
            tones[tone]->nextFireUS = 0;
        }
        else if (deadTones)
        {
            Tone *temp              = tones[tone - deadTones];
            tones[tone - deadTones] = tones[tone];
            tones[tone]             = temp;
        }
    }
    activeTones -= deadTones;
}*/

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
    else
    {
        targetTone       = newTone;
        newTone          = newTone->nextTone;
        if (newTone == firstTone)
        {
            firstTone   = firstTone->nextTone;
        }
        targetTone->type = Tone::Type::newdflt;
    }
    if (periodUS != targetTone->periodUS || ontimeUS != targetTone->ontimeUS)
    {
        targetTone->periodUS = periodUS;
        targetTone->ontimeUS = ontimeUS;
        targetTone->duty     = float(ontimeUS) / float(periodUS);
        targetTone->limitedOntimeUS = ontimeUS;
        limit();
    }
    targetTone->owner    = owner;
    targetTone->origin   = origin;
    return targetTone;
}

void ToneList::deleteTone(Tone* tone)
{
    if (tone)
    {
        tone->ontimeUS   = 0;
        tone->owner      = 0;

        if (tone == firstTone)
        {
            firstTone = firstTone->nextTone;
        }
        else if (tone->nextTone != firstTone)
        {
            tone->prevTone->nextTone       = tone->nextTone;
            tone->nextTone->prevTone       = tone->prevTone;
            tone->nextTone                 = firstTone;
            tone->prevTone                 = firstTone->prevTone;
            tone->prevTone->nextTone       = tone;
            tone->nextTone->prevTone       = tone;
        }
    }
}

void ToneList::buildLinks()
{
    for (uint32_t tone = 0; tone < maxVoices - 1; tone++)
    {
        unorderedTones[tone].nextTone = &(unorderedTones[tone + 1]);
        unorderedTones[tone + 1].prevTone = &(unorderedTones[tone]);
    }
    firstTone = &(unorderedTones[0]);
    firstTone->prevTone = &(unorderedTones[maxVoices - 1]);
    firstTone->prevTone->nextTone = firstTone;
    newTone  = firstTone;
}

void ToneList::setMaxOntimeUS(float maxOntimeUS)
{
    this->maxOntimeUS = maxOntimeUS;
}

void ToneList::setMaxDuty(float maxDuty)
{
    this->maxDuty = maxDuty;
}

void ToneList::setMaxVoices(uint32_t maxVoices)
{
    if (maxVoices > MAX_VOICES)
    {
        maxVoices = MAX_VOICES;
    }
    this->maxVoices = maxVoices;
    buildLinks();
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

uint32_t ToneList::getOntimeUS(uint32_t timeUS)
{
    uint32_t highestOntimeUS = 0;
    Tone* tone = firstTone;
    for (uint32_t toneNum = 0; toneNum < MAX_VOICES; toneNum++)
    {
        if (tone == newTone)
        {
            break;
        }
        if (timeUS >= tone->nextFireUS)
        {
            if (timeUS < tone->nextFireEndUS)
            {
                if (tone->limitedOntimeUS > highestOntimeUS)
                {
                    highestOntimeUS = tone->limitedOntimeUS;
                }
            }
            tone->update(timeUS);
        }
        tone = tone->nextTone;
    }
    return highestOntimeUS;
}
