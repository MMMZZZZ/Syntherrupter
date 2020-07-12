/*
 * Channel.cpp
 *
 *  Created on: 10.05.2020
 *      Author: Max Zuidberg
 */

#include <Channel.h>

Channel::Channel()
{
    resetControllers();
    program = 0;
    coils = 0;
}

Channel::~Channel()
{
    // TODO Auto-generated destructor stub
}

void Channel::resetControllers()
{
    channelAfterTouch = 0;
    volume            = 1.0f;
    expression        = 1.0f;
    pitchBend         = 0.0f;
    modulation        = 0.0f;
    pan               = 0.5f;
    tuning            = 0.0f;
    pitchBendRange    = 2.0f / 8192.0f; // Unit: Semitones / 8192 - since the pitchbend value ranges from -8192 to 8192. Taking that factor in the range reduces the amount of calcs.
    sustainPedal      = false;
    damperPedal       = false;

    pitchBendRangeFine   = 0;
    pitchBendRangeCoarse = 0;
    fineTuningFine       = 0;
    fineTuningCoarse     = 0;
    coarseTuning         = 0;
}

