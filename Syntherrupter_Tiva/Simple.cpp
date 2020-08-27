/*
 * Simple.cpp
 *
 *  Created on: 23.08.2020
 *      Author: Max Zuidberg
 */

#include <Simple.h>


bool Simple::started = false;


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

void Simple::updateToneList()
{
    if (started)
    {
        if (System::getSystemTimeUS() - lastUpdateUS > updatePeriodUS)
        {
            float o = filteredOntimeUS.getFiltered();
            float f = filteredFrequency.getFiltered();
            if (o > 1.0f && f > 1.0f)
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
    else if (tone)
    {
        tonelist->deleteTone(tone);
        tone = 0;
    }
}
