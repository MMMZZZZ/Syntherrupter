/*
 * Coil.h
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#ifndef COIL_H_
#define COIL_H_


#include <stdint.h>
#include <stdbool.h>
#include "InterrupterConfig.h"
#include "Oneshot.h"
#include "Filter.h"
#include "ToneList.h"
#include "MIDI.h"


class Coil
{
public:
    Coil();
    virtual ~Coil();
    void init(uint32_t coilNum);
    void output();
    void setMaxDutyPerm(uint32_t dutyPerm);
    void setMaxOntimeUS(uint32_t ontimeUS);
    void setMaxVoices(uint32_t voices);
    Oneshot  one;
    Filter   filteredOntimeUS;
    Filter   filteredFrequency;
    ToneList toneList;
    MIDI midi;

    uint32_t minOffUS          = 50;
    uint32_t maxOntimeUS       = 10;
    uint32_t maxDutyPerm       = 10;
    uint32_t nextAllowedFireUS = 0;

private:
    uint32_t num = 0;
    void simpleToneUpdate();
};

#endif /* COIL_H_ */
