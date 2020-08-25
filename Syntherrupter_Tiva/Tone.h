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


extern System sys;


class Tone
{
public:
    Tone();
    virtual ~Tone();
    void update(uint32_t timeUS);

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
    uint32_t periodTolUS     = 0;
    uint32_t nextFireUS      = 0;
    uint32_t nextFireEndUS   = 0;
    enum class Type {dflt, rand, newdflt} type = Type::dflt;
};

#endif /* TONE_H_ */
