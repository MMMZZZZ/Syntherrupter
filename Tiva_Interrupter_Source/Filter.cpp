/*
 * Filter.cpp
 *
 *  Created on: 29.03.2020
 *      Author: Max
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

void Filter::init(System* sys, float factor, float constant)
{
    filterSys = sys;
    filterValue = 0.0f;
    filterFactor = factor;
    filterConstant = constant;
    filterMinTimestep = float(filterSys->getSystemTimeResUS()) / 2000000.0f;
}

void Filter::setTarget(float target)
{
    if (filterTarget != target)
    {
        filterTimeUS = filterSys->getSystemTimeUS();
        filterTargetReached = false;
        if (target < 0.0f)
        {
            filterTarget = 0.0f;
        }
        else
        {
            filterTarget = target;
        }
        if (filterTarget > filterValue)
        {
            filterDir = 1;
        }
        else
        {
            filterDir = -1;
        }
    }
}

float Filter::getFiltered()
{
    if (!filterTargetReached)
    {
        uint32_t currentTimeUS = filterSys->getSystemTimeUS();
        float timestep = float(currentTimeUS - filterTimeUS) / 1000000.0f;

        if (timestep > filterMinTimestep)
        {
            filterTimeUS = currentTimeUS;
            timestep *= filterDir;
            filterValue = filterValue * powf(filterFactor, timestep) + filterConstant * timestep;
            if ((filterDir > 0 && filterValue >= filterTarget) || (filterDir < 0 && filterValue <= filterTarget))
            {
                filterValue = filterTarget;
                filterTargetReached = true;
            }
        }
    }

    return filterValue;
}

