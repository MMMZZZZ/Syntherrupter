/*
 * Simple.cpp
 *
 *  Created on: 23.08.2020
 *      Author: Max Zuidberg
 */

#include <Simple.h>


bool Simple::modeRunning = false;


Simple::Simple()
{
    // TODO Auto-generated constructor stub

}

Simple::~Simple()
{
    // TODO Auto-generated destructor stub
}

void Simple::init(ToneList* tonelist, uint32_t updatePeriodUS)
{
    this->tonelist = tonelist;
    this->updatePeriodUS = updatePeriodUS;
}

void Simple::updateToneList()
{
    if (modeRunning)
    {
        if (System::getSystemTimeUS() - lastUpdateUS > updatePeriodUS)
        {
            float o = filteredOntimeUS.getFiltered();
            float f = filteredFrequency.getFiltered();
            f = 1e6f / f;
            tonelist->updateTone<ToneList::Owner::SIMPLE>(0, Tone::Type::dflt, o, f, 0, 0);
            lastUpdateUS = System::getSystemTimeUS();
        }
    }
    else
    {
        tonelist->updateTone<ToneList::Owner::SIMPLE>(0, Tone::Type::dflt, 0, 0, 0, 0);
    }
}
