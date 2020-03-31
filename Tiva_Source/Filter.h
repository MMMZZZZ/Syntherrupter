/*
 * Filter.h
 *
 *  Created on: 29.03.2020
 *      Author: Max
 */

#ifndef FILTER_H_
#define FILTER_H_


#include "System.h"


class Filter
{
public:
    Filter();
    virtual ~Filter();
    void init(System* sys, float factor = 0.05f, uint32_t factorTimeUS = 10000);
    void setTarget(float a);
    float getFiltered();
private:
    System* filterSys;
    uint32_t filterTimeUS       = 0;
    uint32_t filterFactorTimeUS = 0;
    float    filterValue        = 0.0f;
    float    filterFactor       = 0.0f;
    float    filterDirFactor    = 0.0f;
    float    filterTarget       = 0.0f;

};

#endif /* FILTER_H_ */
