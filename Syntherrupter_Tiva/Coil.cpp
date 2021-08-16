/*
 * Coil.cpp
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#include <Coil.h>


Coil Coil::allCoils[COIL_COUNT];
constexpr void (*Coil::allISRs[6])(void);
uint32_t* Coil::bufferDurationUS;


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
    out.init(num, allISRs[num]);
    simple.init(&toneList);
    midi.init(num, &toneList);
    lightsaber.setCoilNum(num);
    lightsaber.setTonelist(&toneList);

    // Correctly apply the settings already loaded by EEPROMSettings
    setMaxDutyPerm(*maxDutyPerm);
    setMaxOntimeUS(*maxOntimeUS);
    setMinOfftimeUS(*minOfftimeUS);
    setMinOntimeUS(*minOntimeUS);
    // yeah the next one is static (thus needs to run only once)
    // buuut having it here is easier and doesn't hurt.
    setBufferDurationUS(*bufferDurationUS);
}

void Coil::setMaxDutyPerm(uint32_t dutyPerm)
{
    toneList.setMaxDuty(dutyPerm / 1000.0f);
    *(this->maxDutyPerm) = dutyPerm;
}

void Coil::setMaxOntimeUS(uint32_t ontimeUS)
{
    out.setMaxOntimeUS(ontimeUS);
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

void Coil::setBufferDurationUS(uint32_t bufferDurationUS)
{
    *Coil::bufferDurationUS = bufferDurationUS;
    Output::setMaxPeriodUS(Branchless::min((*Coil::bufferDurationUS) / 2u, 1000u));
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

void Coil::updateOutput()
{
    uint32_t currentWindowUS = out.lastFiredUS + *bufferDurationUS;
    if (readyForNextUS < currentWindowUS || !out.isActive())
    {
        // Load all pulses for this output. They'll be unsorted
        uint32_t pulseCount = toneList.getOntimesUS(pulses, PULSES_SIZE, readyForNextUS, currentWindowUS);
        if (!pulseCount)
        {
            pulses[0].timeUS = currentWindowUS;
            pulses[0].ontimeUS = 0;
            pulseCount = 1;
        }
        if (pulseCount >= 2)
        {
            // Bring all pulses in chronological order.
            for (int32_t i = pulseCount - 1; i >= 0; i--)
            {
                for (int32_t j = 0; j < i; j++)
                {
                    if (pulses[j].timeUS > pulses[i].timeUS)
                    {
                        float temp = pulses[j].timeUS;
                        pulses[j].timeUS = pulses[i].timeUS;
                        pulses[i].timeUS = temp;
                        temp = pulses[j].ontimeUS;
                        pulses[j].ontimeUS = pulses[i].ontimeUS;
                        pulses[i].ontimeUS = temp;
                    }
                }
            }
        }
        for (uint32_t i = 0; i < pulseCount; i++)
        {
            Pulse& pulse = pulses[i];
            if (pulse.timeUS < readyForNextUS)
            {
                // Ontime is too close to previous ontime; merge them together.
                // Actually, the merge happens in the Output class.
                // Because of the merge, no ontime offset is required (already
                // included in previous ontime).
                pulse.timeUS = lastOntimeEndUS;
            }
            else if (pulse.ontimeUS)
            {
                // "standalone" pulse; add ontime offset
                pulse.ontimeUS += *minOntimeUS;
            }
            readyForNextUS = pulse.timeUS + pulse.ontimeUS;
            pulse.timeUS  -= lastOntimeEndUS;
            out.addPulse(pulse);
            lastOntimeEndUS =  readyForNextUS;
            readyForNextUS += *minOfftimeUS;
        }
    }
    // Prevent overflow issues. Once the smallest variable is above a
    // threshold, all of them are "reset" but substracting that offset.
    static constexpr uint32_t OVERFLOW_THRESHOLD_US = 1 << 30;
    if (out.lastFiredUS > OVERFLOW_THRESHOLD_US)
    {
        out.lastFiredUS -= OVERFLOW_THRESHOLD_US;
        readyForNextUS  -= OVERFLOW_THRESHOLD_US;
        lastOntimeEndUS -= OVERFLOW_THRESHOLD_US;
        toneList.applyTimeOffsetUS(OVERFLOW_THRESHOLD_US);
    }
}
