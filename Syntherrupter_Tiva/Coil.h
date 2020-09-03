/*
 * Coil.h
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#ifndef COIL_H_
#define COIL_H_


#include <stdint.h>
#include <stdbool.h>
#include "InterrupterConfig.h"
#include "Oneshot.h"
#include "ToneList.h"
#include "MIDI.h"
#include "Simple.h"


class Coil
{
public:
    Coil();
    virtual ~Coil();
    void init(uint32_t coilNum);
    void updateData();
    void setMaxDutyPerm(uint32_t dutyPerm);
    void setMaxOntimeUS(uint32_t ontimeUS);
    void setMaxVoices(uint32_t voices);
    void setMinOfftimeUS(uint32_t offtimeUS);
    void updateOutput()
    {
        /*
         * Time critical updates.
         */

        uint32_t timeUS = System::getSystemTimeUS();
        if (timeUS > nextAllowedFireUS)
        {
            nextOntimeUS = toneList.getOntimeUS(timeUS);
            if (nextOntimeUS)
            {
                one.shot(nextOntimeUS);
                nextAllowedFireUS = timeUS + nextOntimeUS + minOffUS;
            }
        }
        /*
         * Overflow detection. No ontime or min offtime is larger than 10 seconds.
         * Note: In theory you'd need a similar detection for the tone.nextFireUS
         * variables. In practice this means that the tones playing during the
         * overflow will stop playing but the next tones will be fine. Therefore
         * it is not worth the additional CPU time to check each time for an
         * overflow. Remember, it only happens less than once an hour.
         */
        else if (nextAllowedFireUS - timeUS > 10000000)
        {
            nextAllowedFireUS = 0;
        }
    };

    Oneshot  one;
    ToneList toneList;
    MIDI     midi;
    Simple   simple;


private:
    uint32_t num               =  0;
    uint32_t minOffUS          = 50;
    uint32_t maxOntimeUS       = 10;
    uint32_t maxDutyPerm       = 10;
    uint32_t nextAllowedFireUS =  0;
    uint32_t nextOntimeUS      =  0;
    uint32_t nextFireUS        = -1;
};

#endif /* COIL_H_ */
