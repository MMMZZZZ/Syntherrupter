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
#include "Output.h"
#include "ToneList.h"
#include "MIDI.h"
#include "Simple.h"
#include "LightSaber.h"


class Coil
{
public:
    Coil();
    virtual ~Coil();
    void init(uint32_t coilNum);
    void updateData();
    void setMaxDutyPerm(uint32_t dutyPerm);
    void setMaxOntimeUS(uint32_t ontimeUS);
    void setMinOfftimeUS(uint32_t offtimeUS);
    void setMinOntimeUS(uint32_t ontimeUS);
    void updateOutput()
    {
        /*
         * Time critical updates.
         */

        uint32_t timeUS = System::getSystemTimeUS();
        if (timeUS > nextAllowedFireUS)
        {
            uint32_t nextOntimeUS = toneList.getOntimeUS(timeUS);
            if (nextOntimeUS)
            {
                nextOntimeUS += *minOntimeUS;
                one.shot(nextOntimeUS);
                nextAllowedFireUS = timeUS + nextOntimeUS + *minOfftimeUS;
            }
        }
        /*
         * Overflow detection. No ontime or min offtime is larger than
         * 10 seconds.
         */
        else if (nextAllowedFireUS - timeUS > 10000000)
        {
            nextAllowedFireUS = 0;
        }
    };

    Oneshot  one;
    Output out;
    ToneList toneList;
    MIDI     midi;
    Simple   simple;
    LightSaber lightsaber;

    static Coil allCoils[COIL_COUNT];
    static void timer0ISR()
    {
        allCoils[0].out.ISR();
    };
    static void timer1ISR()
    {
        allCoils[1].out.ISR();
    };
    static void timer2ISR()
    {
        allCoils[2].out.ISR();
    };
    static void timer3ISR()
    {
        allCoils[3].out.ISR();
    };
    static void timer4ISR()
    {
        allCoils[4].out.ISR();
    };
    static void timer5ISR()
    {
        allCoils[5].out.ISR();
    };
    static constexpr void (*allISRs[6])(void) = {timer0ISR, timer1ISR, timer2ISR, timer3ISR, timer4ISR, timer5ISR};

private:
    uint32_t num =  0;
    uint32_t nextAllowedFireUS =  0;
    // Actual memory (location) provided by EEPROMSettings
    uint32_t* minOntimeUS;
    uint32_t* minOfftimeUS;
    uint32_t* maxOntimeUS;
    uint32_t* maxDutyPerm;
    friend class EEPROMSettings;
};

#endif /* COIL_H_ */
