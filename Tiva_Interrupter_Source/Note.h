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
    uint32_t channel = 0;
    uint32_t number = 0;
    uint32_t velocity = 0;
    uint32_t afterTouch = 0;
    uint32_t ADSRmode = 'A';
    float ADSRontimeUS = 0.0f;
    float ontimeUS = 0.0f;
    float frequency = 0.0f;
    float periodUS = 0.0f;
};

#endif /* NOTE_H_ */
