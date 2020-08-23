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
#include "ToneList.h"
#include "MIDI.h"
#include "Simple.h"


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
    void update();
    Oneshot  one;
    ToneList toneList;
    MIDI midi;
    Simple simple;

    uint32_t minOffUS          = 50;
    uint32_t maxOntimeUS       = 10;
    uint32_t maxDutyPerm       = 10;
    uint32_t nextAllowedFireUS = 0;

private:
    uint32_t num = 0;
};

#endif /* COIL_H_ */
