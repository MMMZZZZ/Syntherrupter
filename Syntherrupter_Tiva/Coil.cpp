/*
 * Coil.cpp
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#include <Coil.h>


Coil Coil::allCoils[COIL_COUNT];


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
    simple.init(&toneList);
    one.init(num);
    midi.init(num, &toneList);
    lightsaber.setCoilNum(num);
    lightsaber.setTonelist(&toneList);

    // Correctly apply the settings already loaded by EEPROMSettings
    setMaxDutyPerm(*maxDutyPerm);
    setMaxOntimeUS(*maxOntimeUS);
    setMinOfftimeUS(*minOfftimeUS);
    setMinOntimeUS(*minOntimeUS);
}

void Coil::setMaxDutyPerm(uint32_t dutyPerm)
{
    toneList.setMaxDuty(dutyPerm / 1000.0f);
    *(this->maxDutyPerm) = dutyPerm;
}

void Coil::setMaxOntimeUS(uint32_t ontimeUS)
{
    one.setMaxOntimeUS(ontimeUS);
    toneList.setMaxOntimeUS(ontimeUS);
    *(this->maxOntimeUS) = ontimeUS;
}

void Coil::setMinOfftimeUS(uint32_t offtimeUS)
{
    *minOfftimeUS = offtimeUS;
}

void Coil::setMinOntimeUS(uint32_t ontimeUS)
{
    *minOntimeUS = ontimeUS;
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
