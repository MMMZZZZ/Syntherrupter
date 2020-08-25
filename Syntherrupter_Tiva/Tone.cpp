/*
 * Tone.cpp
 *
 *  Created on: 16.08.2020
 *      Author: Max Zuidberg
 */

#include <Tone.h>

Tone::Tone()
{
    srand(sys.getSystemTimeUS());
}

Tone::~Tone()
{
    // TODO Auto-generated destructor stub
}

void Tone::update(uint32_t timeUS)
{
    /*
     *  If tone has fired, rearm it. If something changed, update tone.
     */

    switch (type)
    {
        case Type::rand:
        {
            uint32_t freq =  sys.rand(lowerFreq, upperFreq);
            duty          = float(ontimeUS * freq) / 1e6f;
            periodUS      = 1000000 / freq;
            periodTolUS   = periodUS >> periodTolShift;
            break;
        }
        case Type::newdflt:
        {
            periodTolUS   = periodUS >> periodTolShift;
            nextFireUS    = 0;
            type          = Type::dflt;
            break;
        }
    }
    if (nextFireUS)
    {
        // Note that is already playing
        nextFireUS += periodUS;
    }
    else
    {
        // New note
        nextFireUS = timeUS + periodUS;
    }
    nextFireEndUS = nextFireUS + periodTolUS;
}
