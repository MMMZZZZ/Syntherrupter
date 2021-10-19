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

// Forward declaration of Channel class
class Channel;

class Note
{
public:
    Note();
    virtual ~Note();
    bool isDead()
    {
        return (number > 127);
    };
    // For debugging purposes only.
    uint32_t id = 0;

    Note* prevNote = 0;
    Note* nextNote = 0;
    Note* nextChnNote = 0;
    Channel* channel = 0;
    uint32_t envelopeStep    = 0;
    uint8_t number           = 0;
    uint8_t velocity         = 0;
    uint8_t afterTouch       = 0;
    float envelopeTimeUS     = 0.0f;
    float rawVolume          = 0.0f;
    float envelopeVolume     = 0.0f;
    float finishedVolume     = 0.0f;
    float pitch              = 0.0f;
    float frequency          = 0.0f;
    float periodUS           = 0.0f;
    float pan                = 0.5f;
    float panVol[COIL_COUNT]; // Dont like this at all but it was by far the easiest fix.
    uint8_t  panChanged      = (1 << COIL_COUNT) - 1;
    uint8_t toneChanged      = (1 << COIL_COUNT) - 1;
    bool  changed            = true;
    Tone* assignedTones[COIL_COUNT];
};

#endif /* NOTE_H_ */
