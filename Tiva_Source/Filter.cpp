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

void Filter::init(System* sys, float factor, uint32_t factorTimeUS)
{
    filterSys = sys;
    filterValue = 0.0f;
    filterFactor = factor;
    filterFactorTimeUS = factorTimeUS;
    filterTimeUS = filterSys->getSystemTimeUS();
}

void Filter::setTarget(float target)
{
    if (filterTarget != target)
    {
        filterTarget = target;
        if (filterTarget > filterValue)
        {
            filterDirFactor = 1 + filterFactor;
        }
        else
        {
            filterDirFactor = 1 - filterFactor;
        }
        if (filterValue <= 0.0f)
        {
            filterValue = 0.01 * filterTarget;
        }
    }
}

float Filter::getFiltered()
{
    uint32_t currentTimeUS = filterSys->getSystemTimeUS();
    if (currentTimeUS - filterTimeUS >= filterFactorTimeUS)
    {
        filterTimeUS = currentTimeUS;
        if ((filterDirFactor > 1 && filterValue < filterTarget) || (filterDirFactor < 1 && filterValue > filterTarget))
        {
            filterValue *= filterDirFactor;
        }
        //filterValue = filterTarget * filterFactor + filterValue * (1.0f - filterFactor);
    }
    return filterValue;
}

