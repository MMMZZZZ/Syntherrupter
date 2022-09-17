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
#include "Branchless.h"
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
    void applyTimeOffsetUS(uint32_t offsetUS);
    void setMaxOntimeUS(float maxOntimeUS)
    {
        this->maxOntimeUS = maxOntimeUS;
    };
    void setMaxDuty(float maxDuty)
    {
        this->maxDuty = maxDuty;
        limit();
    };
    uint32_t getActiveTones()
    {
        return activeTones;
    };
    uint32_t getSignalDutyPerm()
    {
        return getSignalDuty() * 1e3f;
    };
    float getSignalDuty()
    {
        // When the last tone gets removed the duty limiter is not called
        // and thus signal duty not updated. This is only relevant for the
        // get methods.
        return signalDuty * (activeTones > 0);
    };
    uint32_t getOntimesUS(Pulse* pulses, const uint32_t size, uint32_t nowUS, uint32_t endUS)
    {
        uint32_t index = 0;
        Tone* tone = firstTone;
        while (tone != newTone)
        {
            if (endUS >= tone->nextFireUS)
            {
                pulses[index] = tone->update(nowUS);
                /*
                 * Branchless version of
                 *     if (index < size - 1)
                 *     {
                 *         index++;
                 *     }
                 * No profiling has been done in this case but other (similar)
                 * cases have shown 50-100% speed increases.
                 */
                index += (index < size - 1);
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
    float signalDuty     = 0.0f;
    bool limiterActive   = false;
    uint32_t maxVoices   = MAX_VOICES - 1;
    uint32_t activeTones = 0;
    Tone unorderedTones[MAX_VOICES];
    Tone* newTone;
};

#endif /* TONES_H_ */
