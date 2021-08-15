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
#include <algorithm>
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
    void updateOutput();

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
        GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_1, 0xff);
        allCoils[2].out.ISR();
        GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_1, 0x00);
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
    uint32_t readyForNextUS =  0;
    uint32_t lastOntimeEndUS = 0;
    static constexpr uint32_t PULSES_SIZE = 512;
    uint32_t dataIndex = 0;
    Pulse pulses[PULSES_SIZE];

    // Actual memory (location) provided by EEPROMSettings
    uint32_t* minOntimeUS;
    uint32_t* minOfftimeUS;
    uint32_t* maxOntimeUS;
    uint32_t* maxDutyPerm;
    static uint32_t* bufferDurationUS;
    friend class EEPROMSettings;
};

#endif /* COIL_H_ */
