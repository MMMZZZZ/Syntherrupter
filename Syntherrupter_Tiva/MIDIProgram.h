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


class MIDIProgram
{
public:
    MIDIProgram();
    virtual ~MIDIProgram();
    enum class Mode {lin, exp};
    void setDataPoint(uint32_t index, float amplitude, float durationUS);
    void setMode(Mode mode);
    static constexpr uint32_t DATA_POINTS = 4;
    float durationUS[DATA_POINTS];
    float amplitude[DATA_POINTS];
    float coefficient[DATA_POINTS];
    Mode mode = Mode::lin;
private:
    void updateCoefficients();
};

#endif /* MIDIPROGRAM_H_ */
