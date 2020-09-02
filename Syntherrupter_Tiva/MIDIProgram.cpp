/*
 * MIDIProgram.cpp
 *
 *  Created on: 25.08.2020
 *      Author: Max Zuidberg
 */

#include <MIDIProgram.h>


float MIDIProgram::resolutionUS = 1000.0f;


MIDIProgram::MIDIProgram()
{
    // TODO Auto-generated constructor stub
    for (uint32_t i = 0; i <= DATA_POINTS; i++)
    {
        amplitude[i]   = 1.0f;
        durationUS[i]  = 1.0f;
        ntau[i]        = 3.0f;
    }
    amplitude[DATA_POINTS - 1] = 0.0f;
    mode = Mode::cnst;
    updateCoefficients();
}

MIDIProgram::~MIDIProgram()
{
    // TODO Auto-generated destructor stub
}

void MIDIProgram::setDataPoint(uint32_t index, float amplitude, float durationUS, float ntau)
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

void MIDIProgram::setResolutionUS(float res)
{
    resolutionUS = res;
}

void MIDIProgram::updateCoefficients()
{
    amplitude[DATA_POINTS]   = amplitude[0];
    durationUS[DATA_POINTS]  = durationUS[0];
    ntau[DATA_POINTS]        = ntau[0];
    for (uint32_t i = 1; i <= DATA_POINTS; i++)
    {
        amplitudeDiff[i] = amplitude[i] - amplitude[i - 1];
        if (mode == Mode::lin)
        {
            coefficient[i] = amplitudeDiff[i] / durationUS[i] * resolutionUS;
        }
        else if (mode == Mode::exp)
        {
            coefficient[i]  = expf(- ntau[i] / durationUS[i] * resolutionUS); //powf(expf(-ntau[i]), 1.0f / * ticksPerStep[i]);
            expTargetAmp[i] = amplitude[i - 1] - amplitudeDiff[i] / expm1f(- ntau[i]);
        }
    }
    amplitudeDiff[0] = amplitudeDiff[DATA_POINTS];
    coefficient[0]   = coefficient[DATA_POINTS];
    expTargetAmp[0]  = expTargetAmp[DATA_POINTS];
}
