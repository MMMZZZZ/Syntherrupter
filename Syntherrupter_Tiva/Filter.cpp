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
    this->minTimestepUS = 1000.0f;
}

void Filter::setTarget(float target, bool force)
{
    if (target < 0.0f)
    {
        target = 0.0f;
    }
    if (force)
    {
        value = target;
    }
    if (this->target != target)
    {
        lastTimeUS = System::getSystemTimeUS();
        targetReached = false;
        this->target = target;
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
        uint32_t currentTimeUS = System::getSystemTimeUS();
        float timestep = currentTimeUS - lastTimeUS;

        if (timestep > minTimestepUS)
        {
            lastTimeUS = currentTimeUS;
            timestep /= 1000000.0f;
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

