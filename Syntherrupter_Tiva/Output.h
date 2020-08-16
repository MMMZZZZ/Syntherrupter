/*
 * Output.h
 *
 *  Created on: 28.03.2020
 *      Author: Max Zuidberg
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_


#include <stdint.h>
#include <stdbool.h>
#include "Tone.h"
#include "System.h"


extern System sys;


class Output
{
public:
    Output();
    virtual ~Output();
    void init(uint32_t timerNum);
    void tone(float freq, float ontimeUS);
    void setMaxDuty(float maxDuty);
    void setMaxDutyPerm(uint32_t maxDutyPerm);
    void setMaxOntimeUS(float maxOntimeUS);
    void checkDuty();
private:
};

#endif /* OUTPUT_H_ */
