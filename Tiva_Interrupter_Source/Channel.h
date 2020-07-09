/*
 * Channel.h
 *
 *  Created on: 10.05.2020
 *      Author: Max Zuidberg
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_


#include <stdbool.h>
#include <stdint.h>


class Channel
{
public:
    Channel();
    virtual ~Channel();
    uint32_t channelAfterTouch = 0;
    uint32_t program           = 0;
    uint32_t coils             = 0;
    float pitchBend            = 0.0f;
    float volume               = 1.0f;
    float expression           = 1.0f;
    float modulation           = 0.0f;
    float pan                  = 0.0f;
    float pitchBendRange       = 2.0f / 8192.0f; // Unit: Semitones / 8192 - since the pitchbend value ranges from -8192 to 8192. Taking that factor in the range reduces the amount of calcs.
    float tuning               = 0.0f;
    uint8_t pitchBendRangeFine = 0;
    uint8_t pitchBendRangeCoarse = 0;
    uint8_t fineTuningFine       = 0;
    uint8_t fineTuningCoarse     = 0;
    uint8_t coarseTuning         = 0;
    bool sustainPedal            = false;
    bool damperPedal             = false;
};

#endif /* CHANNEL_H_ */
