/*
 * Filter.cpp
 *
 *  Created on: 29.03.2020
 *      Author: Max Zuidberg
 */

#include <Filter.h>

Filter::Filter()
{
    // TODO Auto-generated constructor stub

}

Filter::~Filter()
{
    // TODO Auto-generated destructor stub
}

void Filter::init(float factor, float constant)
{
    this->value = 0.0f;
    this->factor = factor;
    this->constant = constant;
    this->minTimestep = float(sys.getSystemTimeResUS()) / 2000000.0f;
}

void Filter::setTarget(float target)
{
    if (this->target != target)
    {
        timeUS = sys.getSystemTimeUS();
        targetReached = false;
        if (target < 0.0f)
        {
            this->target = 0.0f;
        }
        else
        {
            this->target = target;
        }
        if (this->target > value)
        {
            dir = 1;
        }
        else
        {
            dir = -1;
        }
    }
}

float Filter::getFiltered()
{
    if (!targetReached)
    {
        uint32_t currentTimeUS = sys.getSystemTimeUS();
        float timestep = float(currentTimeUS - timeUS) / 1000000.0f;

        if (timestep > minTimestep)
        {
            timeUS = currentTimeUS;
            timestep *= dir;
            value = value * powf(factor, timestep) + constant * timestep;
            if ((dir > 0 && value >= this->target) || (dir < 0 && value <= this->target))
            {
                value = this->target;
                targetReached = true;
            }
        }
    }

    return value;
}

