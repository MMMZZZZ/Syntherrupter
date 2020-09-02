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
    void setDataPoint(uint32_t index, float amplitude, float durationUS, float ntau = 1.0f);
    void setMode(Mode mode);
    static void setResolutionUS(float res);
    static constexpr uint32_t DATA_POINTS = 4;
    float durationUS[DATA_POINTS + 1];
    float amplitude[DATA_POINTS + 1];
    float ntau[DATA_POINTS + 1];
    Mode mode = Mode::lin;
    void setADSRAmp(uint32_t* step, float* amp)
    {
        uint32_t currentStep  = *step;
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
        if (  (*amp >= amplitude[currentStep] && amplitudeDiff[currentStep] >= 0)
            ||(*amp <= amplitude[currentStep] && amplitudeDiff[currentStep] <= 0))
        {
            if (currentStep < DATA_POINTS - 2)
            {
                (*step)++;
            }
            *amp = amplitude[currentStep];
        }
    }
private:
    float coefficient[DATA_POINTS + 1];
    float amplitudeDiff[DATA_POINTS + 1];
    float expTargetAmp[DATA_POINTS + 1];
    static float resolutionUS;
void updateCoefficients();
};

#endif /* MIDIPROGRAM_H_ */
