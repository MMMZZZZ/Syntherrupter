/*
 * Coil.cpp
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#include <Coil.h>


Coil::Coil()
{
    // TODO Auto-generated constructor stub
}

Coil::~Coil()
{
    // TODO Auto-generated destructor stub
}

void Coil::init(uint32_t coilNum)
{
    num = coilNum;
    // As of now all coils share the same filter settings. However this is
    // not mandatory.
    simple.init(&toneList, 2.0f, 30.0f, 1.8f, 5.0f);
    one.init(num);
    midi.setCoilNum(num);
    midi.setCoilsToneList(&toneList);
    lightsaber.setTonelist(&toneList);
}

void Coil::setMaxDutyPerm(uint32_t dutyPerm)
{
    toneList.setMaxDuty(dutyPerm / 1000.0f);
}

void Coil::setMaxVoices(uint32_t maxVoices)
{
    midi.setMaxVoices(maxVoices);
}

void Coil::setMaxOntimeUS(uint32_t ontimeUS)
{
    one.setMaxOntimeUS(ontimeUS);
    toneList.setMaxOntimeUS(ontimeUS);
}

void Coil::setMinOfftimeUS(uint32_t offtimeUS)
{
    // Integer ceiling.
    minOffUS = offtimeUS + System::getSystemTimeResUS() - 1;
}

void Coil::updateData()
{
    /*
     * Not time critical updates.
     */

    simple.updateToneList();
    midi.updateToneList();
    lightsaber.updateTonelist();
}
