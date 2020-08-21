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


class Note
{
public:
    Note();
    virtual ~Note();
    uint8_t ADSRMode       = 'A';
    uint8_t channel        = 0;
    uint8_t number         = 0;
    uint8_t velocity       = 0;
    uint8_t afterTouch     = 0;
    float rawOntimeUS       = 0.0f;
    float ADSROntimeUS      = 0.0f;
    float finishedOntimeUS  = 0.0f;
    float frequency         = 0.0f;
    float panVol            = 1.0f;
    bool changed            = false;
};

#endif /* NOTE_H_ */
