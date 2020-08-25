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
#include "Filter.h"
#include "ToneList.h"
#include "System.h"


extern System sys;


class Simple
{
public:
    Simple();
    virtual ~Simple();
    void init(ToneList* tonelist, float ontimeFact, float ontimeConst, float freqFact, float freqConst, uint32_t updatePeriodUS = 10000);
    static void start();
    static void stop();
    void updateToneList();
    void setOntimeUS(float ontimeUS, bool force = false);
    void setFrequency(float periodUS, bool force = false);
private:
    Filter filteredOntimeUS, filteredFrequency;
    ToneList* tonelist;
    Tone* tone;
    uint32_t updatePeriodUS = 10000, lastUpdateUS = 0;
    static bool started;
};

#endif /* SIMPLE_H_ */
