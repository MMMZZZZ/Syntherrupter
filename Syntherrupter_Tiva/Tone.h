/*
 * Tone.h
 *
 *  Created on: 16.08.2020
 *      Author: Max Zuidberg
 */

#ifndef TONE_H_
#define TONE_H_


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "System.h"


class ToneList;

class Tone
{
public:
    Tone();
    virtual ~Tone();
    void setParentList(ToneList* list)
    {
        this->parent = list;
    };
    void update(uint32_t timeUS)
    {

        /*
         *  If tone has fired, rearm it. If something changed, update tone.
         */

        switch (type)
        {
            case Type::rand:
            {
                uint32_t freq =  System::rand(lowerFreq, upperFreq);
                duty          = float(ontimeUS * freq) / 1e6f;
                periodUS      = 1000000 / freq;
                break;
            }
            case Type::newdflt:
            {
                nextFireUS    = 0;
                type          = Type::dflt;
                break;
            }
        }
        if (nextFireUS)
        {
            // Note that is already playing
            nextFireUS += periodUS;
        }
        else
        {
            // New note
            nextFireUS = timeUS + periodUS;
        }
    };
    void remove(void* origin);
    void* owner  = 0;
    void* origin = 0;

    // Noise gen
    uint32_t lowerFreq = 0;
    uint32_t upperFreq = 100;

    // Properties used to generate Output.
    static constexpr uint32_t periodTolShift = 1;
    float    duty            = 0.0f;
    uint32_t ontimeUS        = 0;
    uint32_t limitedOntimeUS = 0;
    uint32_t periodUS        = 0;
    uint32_t nextFireUS      = 0;
    enum class Type {dflt, rand, newdflt} type = Type::dflt;
    Tone* nextTone           = 0;
    Tone* prevTone           = 0;

    // For debugging purposes only.
    uint32_t id = 0;
    ToneList* parent = 0;
};

#endif /* TONE_H_ */
