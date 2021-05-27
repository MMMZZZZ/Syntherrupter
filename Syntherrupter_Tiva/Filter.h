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
    void init(float factor = 2.0f, float constant = 1.0f);
    void setTarget(float target, bool force = false);
    float getFiltered();
    float getTarget()
    {
        return target;
    }
private:
    bool     targetReached = false;
    float    minTimestepUS = 0.0f;
    uint32_t lastTimeUS    = 0;
    float    value         = 0.0f;
    float    dir           = 0.0f;
    float    target        = 0.0f;
    float   *factorPtr;
    float   *constantPtr;
    float   &factor        = *factorPtr;
    float   &constant      = *constantPtr;

    friend class EEPROMSettings;
};

#endif /* FILTER_H_ */
