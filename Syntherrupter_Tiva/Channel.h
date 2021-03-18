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
#include "Note.h"


class Channel
{
public:
    Channel();
    virtual ~Channel();
    void resetControllers();
    void resetNRPs();
    Note* getNote(uint8_t noteNum);
    void addNote(Note* note);
    void removeNote(Note* note);
    void notePanDataUpdate();

    Note* firstNote = 0;
    Note* lastNote  = 0;
    uint32_t noteCount = 0;

    uint32_t number              = 0;
    uint8_t channelAfterTouch    = 0;
    uint8_t program              = 0;
    uint8_t coils                = 0xff; // Startup default: play every channel on every coil.
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
    float notePan                = 0.0f;
    float notePanSourceRangeLow  = 0.0f;
    float notePanSourceRangeHigh = 1.0f;
    float notePanTargetRangeLow  = 0.0f;
    float notePanTargetRangeHigh = 1.0f;
    uint8_t notePanMode          = 0;
    bool sustainPedal            = false;
    bool damperPedal             = false;
    bool controllersChanged      = true;
    bool notesChanged            = false;

    static constexpr uint8_t NOTE_PAN_OFF        = 0;
    static constexpr uint8_t NOTE_PAN_INDIVIDUAL = 1;
    static constexpr uint8_t NOTE_PAN_OMNI       = 2;
    static constexpr uint8_t NOTE_PAN_LOWEST     = 3;
    static constexpr uint8_t NOTE_PAN_HIGHEST    = 4;
    static constexpr uint8_t NOTE_PAN_AVG        = 5;
    static constexpr uint8_t NOTE_PAN_LOUDEST    = 6;
};

#endif /* CHANNEL_H_ */
