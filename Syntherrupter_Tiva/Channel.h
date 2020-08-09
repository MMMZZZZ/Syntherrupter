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
    void resetControllers();
    void resetNRPs();

    uint8_t channelAfterTouch    = 0;
    uint8_t program              = 0;
    uint8_t coils                = 0;
    uint8_t pitchBendRangeFine   = 0;
    uint8_t pitchBendRangeCoarse = 0;
    uint8_t fineTuningFine       = 0;
    uint8_t fineTuningCoarse     = 0;
    uint16_t RPN                 = 0x7f7f;
    uint16_t NRPN                = 0x7f7f;
    uint32_t coarseTuning        = 0;
    float pitchBend              = 0.0f;
    float volume                 = 1.0f;
    float expression             = 1.0f;
    float modulation             = 0.0f;
    float pan                    = 0.0f;
    float pitchBendRange         = 2.0f / 8192.0f; // Unit: Semitones / 8192 - since the pitchbend value ranges from -8192 to 8192. Taking that factor in the range reduces the amount of calcs.
    float tuning                 = 0.0f;
    float notePanSourceRangeLow  = 0.0f;
    float notePanSourceRangeHigh = 1.0f;
    float notePanTargetRangeLow  = 0.0f;
    float notePanTargetRangeHigh = 1.0f;
    bool notePanEnabled          = false;
    bool notePanOmniMode         = false;
    bool sustainPedal            = false;
    bool damperPedal             = false;
};

#endif /* CHANNEL_H_ */
