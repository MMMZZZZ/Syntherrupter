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
#include "LightSaber.h"


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
            uint32_t nextOntimeUS = toneList.getOntimeUS(timeUS);
            if (nextOntimeUS)
            {
                one.shot(nextOntimeUS);
                nextAllowedFireUS = timeUS + nextOntimeUS + minOffUS;
            }
        }
        /*
         * Overflow detection. No ontime or min offtime is larger than
         * 10 seconds.
         */
        else if (nextAllowedFireUS - timeUS > 10000000)
        {
            nextAllowedFireUS = 0;
        }
    };

    static Coil allCoils[COIL_COUNT];
    Oneshot  one;
    ToneList toneList;
    MIDI     midi;
    Simple   simple;
    LightSaber lightsaber;


private:
    uint32_t num               =  0;
    uint32_t minOffUS          = 50;
    uint32_t maxOntimeUS       = 10;
    uint32_t maxDutyPerm       = 10;
    uint32_t nextAllowedFireUS =  0;
};

#endif /* COIL_H_ */
