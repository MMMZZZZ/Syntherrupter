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
    void setDataPoint(uint32_t index, float amplitude, float durationUS, float ntau = 1.0f, uint32_t nextStep = DATA_POINTS);
    void setMode(Mode mode);
    static void setResolutionUS(float res);
    static constexpr uint32_t DATA_POINTS = 8;
    float durationUS[DATA_POINTS];
    float amplitude[DATA_POINTS];
    float ntau[DATA_POINTS];
    uint32_t nextStep[DATA_POINTS];
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
        if (  (*amp >= amplitude[*step] && amplitudeDiff[*step] >= 0)
            ||(*amp <= amplitude[*step] && amplitudeDiff[*step] <= 0))
        {
            *amp  = amplitude[*step];
            *step =  nextStep[*step];

            if (*amp < 1e-6f && (*step == nextStep[*step]))
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
