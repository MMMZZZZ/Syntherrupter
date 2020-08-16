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


extern System sys;


class Filter
{
public:
    Filter();
    virtual ~Filter();
    void init(float factor = 2.0f, float constant = 1.0f);
    void setTarget(float a);
    float getFiltered();
private:
    bool     targetReached = false;
    float    minTimestep   = 0.0f;
    uint32_t timeUS        = 0;
    float    value         = 0.0f;
    float    factor        = 0.0f;
    float    constant      = 0.0f;
    float    dir           = 0.0f;
    float    target        = 0.0f;

};

#endif /* FILTER_H_ */
