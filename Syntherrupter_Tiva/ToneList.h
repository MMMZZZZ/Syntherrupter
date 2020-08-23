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
    uint32_t available();
    Tone* updateTone(uint32_t ontimeUS, uint32_t periodUS, void* owner, void* origin, Tone* tone);
    void deleteTone(Tone* tone);
    void removeDeadTones();
    void setMaxOntimeUS(float ontimeUSLimit);
    void setMaxDuty(float maxDuty);
    void setMaxVoices(uint32_t maxVoices);
    void limit();
    uint32_t getOntimeUS(uint32_t timeUS = 0);
    Tone* tones[MAX_VOICES];

private:
    float maxOntimeUS = 10;
    float maxDuty = 0.01f;
    uint32_t maxVoices = 8;
    uint32_t activeTones = 0;
    Tone unorderedTones[MAX_VOICES];
};

#endif /* TONES_H_ */
