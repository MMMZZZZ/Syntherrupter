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
    setMode(Mode::cnst);
    for (uint32_t i = 0; i < DATA_POINTS; i++)
    {
        setDataPoint(i, 1.0f, 1.0f);
    }
    steps[DATA_POINTS - 1].amplitude = 0.0f;
    steps[DATA_POINTS - 1].nextStep  = DATA_POINTS - 1;
    updateCoefficients();
    setMode(Mode::exp);
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
        steps[index].amplitude = amplitude;
    }
    else
    {
        steps[index].amplitude = 0.0f;
    }

    steps[index].durationUS = durationUS;

    if (mode == Mode::cnst)
    {
        steps[index].ntau = 1e6f;
    }
    else if (mode == Mode::lin)
    {
        steps[index].ntau = 0.0f;
    }
    else if (mode == Mode::exp)
    {
        steps[index].ntau = ntau;
    }

    if (nextStep >= DATA_POINTS)
    {
        if (index == DATA_POINTS - 1 || index == DATA_POINTS - 2)
        {
            nextStep = index;
        }
        else
        {
            nextStep = index + 1;
        }
    }

    steps[index].nextStep = nextStep;

    if (fabsf(steps[index].ntau) < 0.1f)
    {
        steps[index].ntau = 0.1f;
    }

    updateCoefficients();
}

void MIDIProgram::setMode(Mode mode)
{
    if (this->mode != mode)
    {
        this->mode = mode;

        for (uint32_t i = 0; i < DATA_POINTS; i++)
        {
            if (mode == Mode::cnst)
            {
                steps[i].ntau = 1e6f;
            }
            else if (mode == Mode::lin)
            {
                steps[i].ntau = 0.0f;
            }
            else if (mode == Mode::exp)
            {
                steps[i].ntau = 3.0f;
            }
        }
    }
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

    uint32_t currentStep       = 0;
    uint32_t lastDifferentStep = DATA_POINTS - 1;
    uint32_t beforeReleaseStep = DATA_POINTS - 2;
    for (uint32_t i = 0; i < 2 * DATA_POINTS; i++)
    {
        amplitudeDiff[currentStep] = steps[currentStep].amplitude - steps[lastDifferentStep].amplitude;

        coefficient[currentStep]   = expf(- steps[currentStep].ntau / steps[currentStep].durationUS * resolutionUS); // powf(expf(-ntau[currentStep]), 1.0f / * ticksPerStep[currentStep]);
        expTargetAmp[currentStep]  = steps[lastDifferentStep].amplitude - amplitudeDiff[currentStep] / expm1f(- steps[currentStep].ntau);

        // Prevent +/- infinity
        coefficient[currentStep]   = fmaxf(-1e6f, fminf(1e6f, coefficient[currentStep]));
        expTargetAmp[currentStep]  = fmaxf(-1e6f, fminf(1e6f, expTargetAmp[currentStep]));

        stepDone[currentStep] = true;
        if (steps[steps[currentStep].nextStep].amplitude != steps[currentStep].amplitude)
        {
            lastDifferentStep = currentStep;
        }
        if (steps[currentStep].amplitude != steps[DATA_POINTS - 1].amplitude)
        {
            beforeReleaseStep = currentStep;
        }
        currentStep = steps[currentStep].nextStep;
        if (stepDone[currentStep])
        {
            /*
             * A loop has returned to its starting point. All steps of the
             * loop have been calculated. Now calculate the release step
             * based on the last step that is suitable for calculations
             * (meaning the last one that is different from this one).
             */

            if (currentStep != DATA_POINTS - 1)
            {
                currentStep       = DATA_POINTS - 1;
                lastDifferentStep = beforeReleaseStep;
            }
            else
            {
                break;
            }
        }
    }
}
