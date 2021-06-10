/*
 * ToneList.h
 *
 *  Created on: 17.08.2020
 *      Author: Max Zuidberg
 */

#ifndef TONELIST_H_
#define TONELIST_H_


#include <stdint.h>
#include <stdbool.h>
#include "InterrupterConfig.h"
#include "System.h"
#include "Tone.h"
#include "Pulse.h"


class ToneList
{
public:
    ToneList();
    virtual ~ToneList();
    Tone* updateTone(uint32_t ontimeUS, uint32_t periodUS, void* owner, void* origin, Tone* tone);
    void deleteTone(Tone* tone);
    void limit();
    void setMaxOntimeUS(float ontimeUSLimit)
    {
        this->maxOntimeUS = maxOntimeUS;
    };
    void setMaxDuty(float maxDuty)
    {
        this->maxDuty = maxDuty;
        limit();
    };
    uint32_t getOntimesUS(Pulse* pulses, uint32_t length, uint32_t startTimeUS, uint32_t endTimeUS)
    {
        uint32_t index = 0;
        Tone* tone = firstTone;
        while (tone != newTone)
        {
            while (tone->nextFireUS < endTimeUS)
            {
                if (tone->nextFireUS >= startTimeUS && index < length)
                {
                    pulses[index].ontimeUS = tone->limitedOntimeUS;
                    pulses[index].timeUS = tone->nextFireUS;
                    index++;
                }
                tone->update(endTimeUS);
            }
            tone = tone->nextTone;
        }
        return index;
    };
    Tone* firstTone;

private:
    void buildLinks();
    float maxOntimeUS    = 10;
    float maxDuty        = 0.01f;
    bool limiterActive   = false;
    uint32_t maxVoices   = MAX_VOICES - 1;
    uint32_t activeTones = 0;
    Tone unorderedTones[MAX_VOICES];
    Tone* newTone;
};

#endif /* TONES_H_ */
