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
    /*
     * Consider the base e exponential decay
     *   expdecay(t) = amp * exp(-t / tau)
     * With
     *   tau: time constant
     *   amp: start amplitude
     * It reaches the value 1/e after 1*tau.
     *
     * To calculate expdecay iteratively is simple:
     *   amp_n = amp_n-1 * c
     * With
     *   amp_0 = amp
     *   c < 1
     *
     * How to determine c:
     *   c depends on three variables:
     *     amp_0: Stating amplitude
     *     amp_z: Amplitude we want to reach
     *     x:     Number of iterations to reach targetAmp
     *
     *   Based on these values we get the equation
     *     c^x = amp_z / amp_0
     *   Which gives
     *     c = (amp_z / amp_0) ^ (1 / x)
     *
     * At this point we can iteratively calculate an exponential decay with
     *   * any starting amplitude
     *   * any resolution
     *   * any "steepness"
     * However we are still limited to the curve of an exponential decay:
     *   * Reaching/crossing 0 is impossible
     *   * Always goes towards 0.
     *
     * Remember how we defined amp_z: It's the value we reach after x
     * iterations, it is not the limit of the series, which will remain 0.
     * What would be much more useful is to scale the curve such that it
     * crosses zero after x steps. This of course increases the total
     * scale (lets call it totalScale).
     *
     * TODO more explanations at this point, especially considering ntau
     *
     * Solution:
     *   amp_n = (amp_n - totalScale) * c + totalScale
     *
     */
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
