/*
 * Simple.h
 *
 *  Created on: 23.08.2020
 *      Author: Max Zuidberg
 */

#ifndef SIMPLE_H_
#define SIMPLE_H_


#include <stdint.h>
#include <stdbool.h>
#include "System.h"
#include "Filter.h"
#include "ToneList.h"


class Simple
{
public:
    Simple();
    virtual ~Simple();
    void init(ToneList* tonelist, uint32_t updatePeriodUS = 10000);
    void updateToneList();
    static void start()
    {
        started = true;
    };
    static void stop()
    {
        started = false;
    };
    void setOntimeUS(float ontimeUS, bool force = false)
    {
        filteredOntimeUS.setTarget(ontimeUS, force);
    };
    void setDuty(float duty, bool force = false)
    {
        float ontimeUS = duty * 1e6f / frequency;
        filteredOntimeUS.setTarget(ontimeUS, force);
    };
    void setFrequency(float freq, bool force = false)
    {
        frequency = freq;
        filteredFrequency.setTarget(freq, force);
    };
    float getDuty()
    {
        return filteredOntimeUS.getTarget() * filteredFrequency.getTarget() / 1e6f;
    };
    float getOntimeUS()
    {
        return filteredOntimeUS.getTarget();
    };
    float getFrequency()
    {
        return filteredFrequency.getTarget();
    };
private:
    Filter filteredOntimeUS, filteredFrequency;
    ToneList* tonelist;
    Tone* tone;
    uint32_t updatePeriodUS = 10000, lastUpdateUS = 0;
    float frequency = 0.0f;
    static bool started;

    friend class EEPROMSettings;
};

#endif /* SIMPLE_H_ */
