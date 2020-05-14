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
    volatile uint32_t channel        = 0;
    volatile uint32_t number         = 0;
    volatile uint32_t velocity       = 0;
    volatile uint32_t afterTouch     = 0;
    volatile uint32_t periodUS       = 0;
    volatile uint32_t halfPeriodUS   = 0;
    volatile uint32_t ADSRMode       = 'A';
    volatile float rawOntimeUS       = 0.0f;
    volatile float ADSROntimeUS      = 0.0f;
    volatile float finishedOntimeUS  = 0.0f;
    volatile float frequency         = 0.0f;
    volatile bool fired              = false;
};

#endif /* NOTE_H_ */
