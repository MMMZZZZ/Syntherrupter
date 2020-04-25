/*
 * Coil.h
 *
 *  Created on: 25.04.2020
 *      Author: Max
 */

#ifndef COIL_H_
#define COIL_H_


#include <stdint.h>
#include "Output.h"
#include "MIDI.h"
#include "Filter.h"


class Coil
{
public:
    Coil();
    virtual ~Coil();
    MIDI     midi;
    Output   out;
    Filter   filteredOntimeUS;
    Filter   filteredFrequency;

    uint32_t maxDutyPerc = 1;
    uint32_t maxOntimeUS = 1;
    uint32_t ontimeUS = 0;
    uint32_t periodUS = 0;
};

#endif /* COIL_H_ */
