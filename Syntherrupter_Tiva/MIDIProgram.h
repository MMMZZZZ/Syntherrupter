/*
 * MIDIProgram.h
 *
 *  Created on: 25.08.2020
 *      Author: Max Zuidberg
 */

#ifndef MIDIPROGRAM_H_
#define MIDIPROGRAM_H_


#include <stdint.h>
#include <stdbool.h>
#include <math.h>


class MIDIProgram
{
public:
    MIDIProgram();
    virtual ~MIDIProgram();
    enum class Mode {lin, exp, cnst};
    void setDataPoint(uint32_t index, float amplitude, float durationUS);
    void setMode(Mode mode);
    static void setResolutionUS(float res);
    static constexpr uint32_t DATA_POINTS = 4;
    float durationUS[DATA_POINTS + 1];
    float amplitude[DATA_POINTS + 1];
    float ntau[DATA_POINTS + 1];
    Mode mode = Mode::lin;
    void setADSRAmp(uint32_t* step, uint32_t* ticks, float* amp)
    {
        uint32_t currentStep  = *step;
        /*uint32_t previousStep = currentStep;
        if (previousStep)
        {
            previousStep--;
        }
        else
        {
            previousStep = DATA_POINTS - 1;
        }*/
        if (*ticks >= ticksPerStep[currentStep])
        {
            if (currentStep < DATA_POINTS - 2)
            {
                *ticks = 0;
                (*step)++;
            }
            *amp = amplitude[currentStep];
            return;
        }
        else
        {
            (*ticks)++;
        }
        if (mode == Mode::lin)
        {
            *amp += coefficient[currentStep];
        }
        else if (mode == Mode::exp)
        {
            *amp -= expTargetAmp[currentStep];
            *amp *= coefficient[currentStep];
            *amp += expTargetAmp[currentStep];
        }
        else //if (mode == Mode::cnst)
        {
            *amp = amplitude[currentStep];
        }
    }
private:
    float coefficient[DATA_POINTS + 1];
    float amplitudeDiff[DATA_POINTS + 1];
    float expTargetAmp[DATA_POINTS + 1];
    static float resolutionUS;
    uint32_t ticksPerStep[DATA_POINTS + 1];
void updateCoefficients();
};

#endif /* MIDIPROGRAM_H_ */
