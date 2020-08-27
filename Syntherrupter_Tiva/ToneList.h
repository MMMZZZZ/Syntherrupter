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
    void setMaxOntimeUS(float ontimeUSLimit);
    void setMaxDuty(float maxDuty);
    void setMaxVoices(uint32_t maxVoices);
    void limit();
    uint32_t getOntimeUS(uint32_t timeUS);
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
