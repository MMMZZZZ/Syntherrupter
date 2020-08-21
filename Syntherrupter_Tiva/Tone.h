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

    // Properties required by MIDI class
    uint8_t ADSRMode       = 'A';
    uint8_t channel        = 0;
    uint8_t number         = 0;
    uint8_t velocity       = 0;
    uint8_t afterTouch     = 0;
    float rawOntimeUS       = 0.0f;
    float ADSROntimeUS      = 0.0f;
    float frequency         = 0.0f;
    float panVol            = 1.0f;
    bool changed            = false;

    // Noise gen
    uint32_t lowerFreq = 0;
    uint32_t upperFreq = 100;

    // Properties used to generate Output.
    uint32_t periodTolShift = 1;
    uint32_t ontimeUS       = 0;
    uint32_t periodUS       = 0;
    uint32_t periodTolUS    = 0;
    uint32_t nextFireUS     = 0;
    uint32_t nextFireEndUS  = 0;
    enum class Type {dflt, rand} type = Type::dflt;
};

#endif /* TONE_H_ */
