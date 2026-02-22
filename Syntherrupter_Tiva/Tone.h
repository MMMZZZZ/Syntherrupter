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
#include "Branchless.h"
#include "System.h"
#include "Pulse.h"


class ToneList;

class Tone
{
public:
    Tone();
    virtual ~Tone();
    Pulse update(uint32_t timeUS)
    {

        /*
         *  If tone has fired, rearm it. If something changed, update tone.
         */

        // New tone => make sure it fires by bringing nextFireUS up to date
        // else leave it as-is.
        nextFireUS = Branchless::selectByCond(timeUS, nextFireUS, isNew);

        switch (type)
        {
            case Type::rand:
            {
                uint32_t freq = System::rand(lowerFreq, upperFreq);
                duty          = float(ontimeUS * freq) / 1e6f;
                periodUS      = 1000000 / freq;
                break;
            }
            case Type::dflt:
            {
                // No special actions required
                break;
            }
        }

        Pulse pulse;
        pulse.ontimeUS = limitedOntimeUS;
        pulse.timeUS   = nextFireUS;

        nextFireUS += periodUS;

        isNew = false;

        return pulse;
    };
    void* owner  = 0;
    void* origin = 0;

    // Noise gen
    uint32_t lowerFreq = 0;
    uint32_t upperFreq = 100;

    // Properties used to generate Output.
    static constexpr uint32_t periodTolShift = 1;
    bool     isNew           = true;
    float    duty            = 0.0f;
    float    minDuty         = 0.0f;
    uint32_t ontimeUS        = 0;
    uint32_t limitedOntimeUS = 0;
    uint32_t periodUS        = 0;
    uint32_t nextFireUS      = 0;
    enum class Type {dflt, rand} type = Type::dflt;
};

#endif /* TONE_H_ */
