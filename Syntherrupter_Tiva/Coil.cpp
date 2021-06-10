/*
 * Coil.cpp
 *
 *  Created on: 25.04.2020
 *      Author: Max Zuidberg
 */

#include <Coil.h>


Coil Coil::allCoils[COIL_COUNT];
constexpr void (*Coil::allISRs[6])(void);


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

void Coil::updateOutput()
{
    if (out.requiresData())
    {
        uint32_t bufferStartTimeUS = bufferEndTimeUS;
        bufferEndTimeUS += *bufferDurationUS;

        // Load all pulses for this output. They'll be unsorted but guaranteed
        // to have timeUS < bufferEndTimeUS
        uint32_t pulseCount = toneList.getOntimesUS(pulses, PULSES_SIZE, bufferStartTimeUS, bufferEndTimeUS);
        if (pulseCount)
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

            /*
             *  * Remove all pulses that violate the min offtime.
             *  * Adjust the pulse.timeUS to be relative to the start time of this
             *    buffer.
             *  * Limit ontimes that are too high.
             *
             */
            for (uint32_t i = 0; i < pulseCount; i++)
            {
                if (pulses[i].timeUS > nextAllowedFireUS && pulses[i].ontimeUS >= 2)
                {
                    if (pulses[i].ontimeUS > *maxOntimeUS)
                    {
                        pulses[i].ontimeUS = *maxOntimeUS;
                    }
                    nextAllowedFireUS = pulses[i].timeUS + pulses[i].ontimeUS + *minOfftimeUS;
                }
                else
                {
                    pulses[i].ontimeUS = 0;
                }
                pulses[i].timeUS -= bufferStartTimeUS;
            }

            // Adjust the bufferEndTime to match the actual last pulse in the
            // buffer.
            bufferEndTimeUS = bufferStartTimeUS + pulses[pulseCount - 1].timeUS + pulses[pulseCount - 1].ontimeUS;
        }
        else
        {
            pulses[0].timeUS = *bufferDurationUS - 1;
            pulses[0].ontimeUS = 0;
            pulseCount = 1;
        }
        // Pulse list done, hand it to the output timer
        out.insert(pulses, pulseCount, *bufferDurationUS);
        /*
         * Overflow detection. From the lines above, nextAllowedFireUS
         * has to be greater or equal to bufferEndTimeUS - except if
         * an overflow occured.
         */
        if (nextAllowedFireUS < bufferEndTimeUS)
        {
            nextAllowedFireUS = bufferEndTimeUS;
        }


        if (++dataIndex >= 100)
        {
            dataIndex = 0;
            if (num == 2)
            {
                dataIndex = 0;
            }
        }
        pulses = pulseData[dataIndex];
    }
}
