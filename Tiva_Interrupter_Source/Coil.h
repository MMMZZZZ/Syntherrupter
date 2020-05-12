/*
 * Coil.h
 *
 *  Created on: 25.04.2020
 *      Author: Max
 */

#ifndef COIL_H_
#define COIL_H_


#include <stdint.h>
#include <stdbool.h>
#include "InterrupterConfig.h"
#include "Output.h"
#include "Oneshot.h"
#include "Filter.h"


class Coil
{
public:
    Coil();
    virtual ~Coil();
    Output   out;
    Oneshot  one;
    Filter   filteredOntimeUS;
    Filter   filteredFrequency;

    uint32_t ontimeUS = 0;
    uint32_t periodUS = 0;
};

#endif /* COIL_H_ */
