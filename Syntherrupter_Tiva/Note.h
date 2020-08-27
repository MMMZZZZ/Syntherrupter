/*
 * Note.h
 *
 *  Created on: 08.05.2020
 *      Author: Max Zuidberg
 */

#ifndef NOTE_H_
#define NOTE_H_


#include <stdbool.h>
#include <stdint.h>
#include "InterrupterConfig.h"
#include "Tone.h"


class Note
{
public:
    Note();
    virtual ~Note();
    bool isDead()
    {
        return ((ADSRStep && ADSRVolume < 0.1f) || number > 127);
    };
    uint32_t ADSRStep      = 0;
    uint8_t channel        = 0;
    uint8_t number         = 0;
    uint8_t velocity       = 0;
    uint8_t afterTouch     = 0;
    float ADSRTimeUS        = 0.0f;
    float rawVolume         = 0.0f;
    float ADSRVolume        = 0.0f;
    float finishedVolume    = 0.0f;
    float frequency         = 0.0f;
    float pan               = 0.5f;
    float panVol[COIL_COUNT]; // Dont like this at all but it was by far the easiest fix.
    uint8_t  panChanged     = COIL_COUNT;
    bool  changed           = true;
    Tone* assignedTones[COIL_COUNT];
    Note* prevNote = 0;
    Note* nextNote = 0;
};

#endif /* NOTE_H_ */
