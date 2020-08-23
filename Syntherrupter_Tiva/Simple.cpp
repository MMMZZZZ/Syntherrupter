/*
 * Simple.cpp
 *
 *  Created on: 23.08.2020
 *      Author: Max
 */

#include <Simple.h>

Simple::Simple()
{
    // TODO Auto-generated constructor stub

}

Simple::~Simple()
{
    // TODO Auto-generated destructor stub
}

void Simple::init(ToneList* tonelist, float ontimeFact, float ontimeConst, float freqFact, float freqConst, uint32_t updatePeriodUS)
{
    this->tonelist = tonelist;
    this->updatePeriodUS = updatePeriodUS;
    filteredOntimeUS.init(ontimeFact, ontimeConst);
    filteredFrequency.init(freqFact, freqConst);
}

void Simple::setOntimeUS(float ontimeUS, bool force)
{
    filteredOntimeUS.setTarget(ontimeUS, force);
}

void Simple::setFrequency(float freq, bool force)
{
    filteredFrequency.setTarget(freq, force);
}


void Simple::updateToneList()
{
    if (sys.getSystemTimeUS() - lastUpdateUS > updatePeriodUS)
    {
        float o = filteredOntimeUS.getFiltered();
        float f = filteredFrequency.getFiltered();
        if (o > 1.0f)
        {
            f = 1e6f / f;
            tone = tonelist->updateTone(o, f, this, this, tone);
        }
        else
        {
            tonelist->deleteTone(tone);
            tone = 0;
        }
    }
}
