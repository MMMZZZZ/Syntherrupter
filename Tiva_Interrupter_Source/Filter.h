/*
 * Filter.h
 *
 *  Created on: 29.03.2020
 *      Author: Max Zuidberg
 */

#ifndef FILTER_H_
#define FILTER_H_


#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "System.h"


class Filter
{
public:
    Filter();
    virtual ~Filter();
    void init(System* sys, float factor = 2.0f, float constant = 1.0f);
    void setTarget(float a);
    float getFiltered();
private:
    System*  filterSys;
    bool     filterTargetReached = false;
    float    filterMinTimestep   = 0.0f;
    uint32_t filterTimeUS        = 0;
    float    filterValue         = 0.0f;
    float    filterFactor        = 0.0f;
    float    filterConstant      = 0.0f;
    float    filterDir     = 0.0f;
    float    filterTarget        = 0.0f;

};

#endif /* FILTER_H_ */
