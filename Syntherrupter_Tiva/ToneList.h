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


extern System sys;


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
    void setMaxVoices(uint32_t maxVoices)
    {
        if (maxVoices > MAX_VOICES)
        {
            maxVoices = MAX_VOICES;
        }
        this->maxVoices = maxVoices;
        buildLinks();
    };
    uint32_t getOntimeUS(uint32_t timeUS)
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
    };
    Tone* firstTone;

private:
    void buildLinks();
    float maxOntimeUS    = 10;
    float maxDuty        = 0.01f;
    bool limiterActive   = false;
    uint32_t maxVoices   = 8;
    uint32_t activeTones = 0;
    Tone unorderedTones[MAX_VOICES];
    Tone* newTone;
};

#endif /* TONES_H_ */
