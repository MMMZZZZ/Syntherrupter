/*
 * MIDIProgram.cpp
 *
 *  Created on: 25.08.2020
 *      Author: Max Zuidberg
 */

#include <MIDIProgram.h>



MIDIProgram::MIDIProgram()
{
    // TODO Auto-generated constructor stub
    for (uint32_t i = 0; i < DATA_POINTS; i++)
    {
        amplitude[i]   = 1.0f;
        durationUS[i]  = 1.0f;
        coefficient[i] = 1.0f;
    }
}

MIDIProgram::~MIDIProgram()
{
    // TODO Auto-generated destructor stub
}

void MIDIProgram::setDataPoint(uint32_t index, float amplitude, float durationUS)
{
    if (index >= DATA_POINTS)
    {
        return;
    }
    this->amplitude[index]  = amplitude;
    this->durationUS[index] = durationUS;
    updateCoefficients();
}

void MIDIProgram::setMode(Mode mode)
{
    this->mode = mode;
    updateCoefficients();
}

void MIDIProgram::updateCoefficients()
{
    float lastAmp = amplitude[DATA_POINTS - 1];
    for (uint32_t i = 0; i < DATA_POINTS; i++)
    {
        if (mode == Mode::lin)
        {
            coefficient[i] = (amplitude[i] - lastAmp) / durationUS[i];
            lastAmp = amplitude[i];
        }
    }
}
