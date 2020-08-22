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
        tones[tone] = &(unorderedTones[tone]);
    }
    newTone = tones[0];
    simpleTone = 0;
}

ToneList::~ToneList()
{
    // TODO Auto-generated destructor stub
}

void ToneList::removeDeadTones()
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
}

uint32_t ToneList::available()
{
    return (MAX_VOICES - activeTones);
}

Tone* ToneList::updateTone(uint32_t ontimeUS, uint32_t periodUS, void* owner, void* origin, Tone* tone)
{
    Tone* targetTone = 0;
    if (tone)
    {
        if ((uint32_t) tone >= (uint32_t) tones && (uint32_t) tone <= (uint32_t) tones + MAX_VOICES)
        {
            targetTone = tone;
        }
    }
    else if (activeTones < MAX_VOICES)
    {
        newToneIndex     = activeTones++;
        targetTone       = tones[activeTones];
        targetTone->type = Tone::Type::newdflt;
    }
    if (targetTone)
    {
        targetTone->ontimeUS = ontimeUS;
        targetTone->periodUS = periodUS;
        targetTone->owner    = owner;
        targetTone->origin   = origin;
        return targetTone;
    }
    return 0;
}

void ToneList::deleteTone(Tone* tone)
{
    if ((uint32_t) tone >= (uint32_t) tones && (uint32_t) tone <= (uint32_t) tones + MAX_VOICES)
    {
        tone->ontimeUS = 0;
        removeDeadTones();
    }
}

void ToneList::saveNewTone()
{
    if (activeTones < MAX_VOICES)
    {
        newToneIndex = ++activeTones;
    }
    newTone = tones[activeTones];
}

void ToneList::setMaxOntimeUS(float maxOntimeUS)
{
    this->maxOntimeUS = maxOntimeUS;
}

void ToneList::setMaxDuty(float maxDuty)
{
    this->maxDuty = maxDuty;
}

void ToneList::limit()
{
    float totalDuty = 0.0f;
    for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
    {
        if (tone < activeTones)
        {
            float ontimeUS = tones[tone]->ontimeUS;
            if (ontimeUS > maxOntimeUS)
            {
                tones[tone]->ontimeUS = maxOntimeUS;
                ontimeUS = maxOntimeUS;
            }
            totalDuty += ontimeUS / float(tones[tone]->periodUS);
        }
    }
    if (totalDuty > maxDuty)
    {
        // Duty of all notes together exceeds coil limit; reduce ontimes.

        // Factor by which ontimes must be reduced.
        totalDuty = maxDuty / totalDuty;
        for (uint32_t tone = 0; tone < activeTones; tone++)
        {
            tones[tone]->ontimeUS *= totalDuty;
        }
    }
}

void ToneList::setSimpleTone(float ontimeUS, float frequency)
{
    if (!ontimeUS)
    {
        if (simpleTone)
        {
            simpleTone->nextFireUS = 0;
            removeDeadTones();
        }
    }
    else
    {
        if (!simpleTone)
        {
            simpleTone = newTone;
            saveNewTone();
        }
        simpleTone->periodUS = 1e6f / frequency;
        simpleTone->ontimeUS = ontimeUS;
        simpleTone->update(sys.getSystemTimeUS());
    }
}

uint32_t ToneList::getOntimeUS(uint32_t timeUS)
{
    if (!timeUS)
    {
        timeUS = sys.getSystemTimeUS();
    }
    uint32_t highestOntimeUS = 0;
    for (uint32_t tone = 0; tone < MAX_VOICES; tone++)
    {
        if (tone < activeTones)
        {
            Tone *currentTone = tones[tone];
            if (timeUS >= currentTone->nextFireUS)
            {
                if (timeUS < currentTone->nextFireEndUS)
                {
                    if (currentTone->ontimeUS > highestOntimeUS)
                    {
                        highestOntimeUS = currentTone->ontimeUS;
                    }
                }
                currentTone->update(timeUS);
            }
        }
    }
    return highestOntimeUS;
}
