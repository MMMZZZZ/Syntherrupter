/*
 * Coil.cpp
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#include <Coil.h>

Coil::Coil()
{
    // TODO Auto-generated constructor stub
}

Coil::~Coil()
{
    // TODO Auto-generated destructor stub
}

void Coil::init(uint32_t coilNum)
{
    num = coilNum;
    // As of now all coils share the same filter settings. However this is
    // not mandatory.
    filteredFrequency.init(1.8f, 5.0f);
    filteredOntimeUS.init(2.0f, 30.0f);
    one.init(num);
    midi.setCoilNum(num);
    midi.setCoilsToneList(&toneList);
}

void Coil::setMaxDutyPerm(uint32_t dutyPerm)
{
    toneList.setMaxDuty(dutyPerm / 1000.0f);
}

void Coil::setMaxVoices(uint32_t maxVoices)
{
}

void Coil::setMaxOntimeUS(uint32_t ontimeUS)
{
    one.setMaxOntimeUS(ontimeUS);
    toneList.setMaxOntimeUS(ontimeUS);
}

void Coil::simpleToneUpdate()
{
    float o = filteredOntimeUS.getFiltered();
    float f = filteredFrequency.getFiltered();
    toneList.setSimpleTone(o, f);
}

void Coil::output()
{
    uint32_t timeUS = sys.getSystemTimeUS();
    if (timeUS > nextAllowedFireUS)
    {
        uint32_t ontimeUS = toneList.getOntimeUS(timeUS);
        if (ontimeUS)
        {
            one.shot(ontimeUS);
            nextAllowedFireUS = timeUS + ontimeUS + minOffUS;
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
}
