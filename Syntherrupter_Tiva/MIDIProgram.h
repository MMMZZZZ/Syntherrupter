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
    struct DataPoint
    {
        uint8_t nextStep;
        float durationUS;
        float amplitude;
        float ntau;
    };
    MIDIProgram();
    virtual ~MIDIProgram();
    enum class Mode {lin, exp, cnst};
    void setDataPoint(uint32_t index, float amplitude, float durationUS, float ntau = 1.0f, uint32_t nextStep = DATA_POINTS);
    void setMode(Mode mode);
    static void setResolutionUS(float res);
    static constexpr uint32_t DATA_POINTS = 8;

    // Actual memory location provided by EEPROMSettings
    DataPoint (*stepsPointer)[DATA_POINTS];
    DataPoint (&steps)[DATA_POINTS] = (*stepsPointer);

    Mode mode = Mode::exp;
    bool setEnvelopeAmp(uint32_t* step, float* amp)
    {
        // Return if note is still active or not.

        /*if (mode == Mode::lin)
        {
            *amp += coefficient[*step];
        }
        else if (mode == Mode::exp)
        {*/
            *amp -= expTargetAmp[*step];
            *amp *= coefficient[*step];
            *amp += expTargetAmp[*step];
        /*}
        else //if (mode == Mode::cnst)
        {
            *amp = amplitude[*step];
        }*/
        if (  (*amp >= steps[*step].amplitude && amplitudeDiff[*step] >= 0)
            ||(*amp <= steps[*step].amplitude && amplitudeDiff[*step] <= 0))
        {
            *amp  = steps[*step].amplitude;
            *step = steps[*step].nextStep;

            if (*amp < 1e-6f && (*step == steps[*step].nextStep))
            {
                // No amplitude left and it will stay like this. A.k.a. note ended.
                return false;
            }
        }
        return true;
    }
private:
    float coefficient[DATA_POINTS];
    float amplitudeDiff[DATA_POINTS];
    float expTargetAmp[DATA_POINTS];
    static float resolutionUS;
    void updateCoefficients();
};

#endif /* MIDIPROGRAM_H_ */
