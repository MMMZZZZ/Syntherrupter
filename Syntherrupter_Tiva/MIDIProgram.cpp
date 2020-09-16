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

void MIDIProgram::setDataPoint(uint32_t index, float amplitude, float durationUS, float ntau, uint32_t nextStep)
{
    if (index >= DATA_POINTS)
    {
        return;
    }
    if (index != DATA_POINTS - 1)
    {
        // Amplitude of the release datapoint cannot be set to another value
        // than the default 0.0f.
        this->amplitude[index]  = amplitude;
    }
    this->durationUS[index] = durationUS;
    this->ntau[index]       = ntau;
    if (index == DATA_POINTS - 1)
    {
        nextStep = DATA_POINTS - 1;
    }
    else if (nextStep == DATA_POINTS)
    {
        nextStep = index + 1;
    }
    this->nextStep[index]   = nextStep;
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
     * Note: This could be simplified but would become even less readable.
     *         amp_n = amp_n * c + (totalScale * (1 - c))
     *       Since the stuff in the brackets is completely constant, it can
     *       be calculated ahead. This would reduce the "runtime calculations"
     *       from 3 (sub mul add) to 2 (mul add).
     */

    bool stepDone[DATA_POINTS];
    for (uint32_t i = 0; i < DATA_POINTS; i++)
    {
        stepDone[i] = false;
    }
    uint32_t currentStep = 0;
    uint32_t lastStep    = DATA_POINTS - 1;
    do
    {
        if (stepDone[currentStep])
        {
            /* A loop has returned to its starting point. All steps of the
             * loop have been calculated. Now calculate the release step
             * depending on how the loop "ends".
             */

            if (amplitude[lastStep] == 0.0f)
            {
                /*
                 * The last step of the loop reaches 0. So the release should
                 * act just like this data point. (Additionally that last step
                 * cant be used to determine the release step since the
                 * amplitude difference is 0).
                 */
                coefficient[DATA_POINTS - 1]  = coefficient[lastStep];
                expTargetAmp[DATA_POINTS - 1] = expTargetAmp[lastStep];
                break;
            }
            else
            {
                /*
                 * The last step of the loop does not reach 0. Use this step
                 * as previous step to the release.
                 */
                currentStep = DATA_POINTS - 1;
            }
        }

        amplitudeDiff[currentStep] = amplitude[currentStep] - amplitude[lastStep];
        if (mode == Mode::lin)
        {
            coefficient[currentStep] = amplitudeDiff[currentStep] / durationUS[currentStep] * resolutionUS;
        }
        else if (mode == Mode::exp)
        {
            coefficient[currentStep]  = expf(- ntau[currentStep] / durationUS[currentStep] * resolutionUS); // powf(expf(-ntau[currentStep]), 1.0f / * ticksPerStep[currentStep]);
            expTargetAmp[currentStep] = amplitude[lastStep] - amplitudeDiff[currentStep] / expm1f(- ntau[currentStep]);
        }

        stepDone[currentStep] = true;
        lastStep = currentStep;
        currentStep = nextStep[currentStep];

    } while (lastStep != DATA_POINTS - 1);
}
