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
    };
    uint32_t getOntimeUS(uint32_t timeUS)
    {
        Tone* tone = firstTone;
        while (tone != newTone)
        {
            if (timeUS >= tone->nextFireUS)
            {
                tone->update(timeUS);
                return tone->limitedOntimeUS;
            }
            tone = tone->nextTone;
        }
        return 0;
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
