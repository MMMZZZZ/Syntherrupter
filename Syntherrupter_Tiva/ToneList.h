/*
 * ToneList.hpp
 *
 *  Created on: 21.02.2026
 *      Author: Max Zuidberg
 */

#ifndef TONELIST_H_
#define TONELIST_H_


#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "InterrupterConfig.h"
#include "Branchless.h"
#include "System.h"
#include "Tone.h"
#include "Pulse.h"


class ToneList
{
public:
    ToneList();
    virtual ~ToneList();

    enum class Owner {
        SIMPLE,
        MIDI_LIVE,
        LIGHTSABER,
    };

    template<Owner owner> void updateTone(uint32_t index, Tone::Type type, uint32_t ontimeUS, uint32_t periodUS, uint32_t lowerFreq, uint32_t upperFreq);
    template<Owner owner> uint32_t getFreeTone();
    void limit();
    void applyTimeOffsetUS(uint32_t offsetUS);
    void setMinOntimeUS(float minOntimeUS);
    void setMaxOntimeUS(float maxOntimeUS);
    void setMaxDuty(float maxDuty)
    {
        this->maxDuty = maxDuty;
        limit();
    };
    uint32_t getActiveTones()
    {
        return activeTones;
    };
    uint32_t getSignalDutyPerm()
    {
        return getSignalDuty() * 1e3f;
    };
    float getSignalDuty()
    {
        // When the last tone gets removed the duty limiter is not called
        // and thus signal duty not updated. This is only relevant for the
        // get methods.
        return signalDuty * (activeTones > 0);
    };
    uint32_t getOntimesUS(Pulse* pulses, const uint32_t size, uint32_t nowUS, uint32_t endUS)
    {
        uint32_t index = 0;
        for (uint32_t toneNum = 0; toneNum < TONE_COUNT_TOTAL; toneNum++)
        {
            auto& tone = tonelist[toneNum];
            if (!tone.limitedOntimeUS)
            {
                continue;
            }
            if (endUS >= tone.nextFireUS)
            {
                pulses[index] = tone.update(nowUS);
                /*
                 * Branchless version of
                 *     if (index < size - 1)
                 *     {
                 *         index++;
                 *     }
                 * No profiling has been done in this case but other (similar)
                 * cases have shown 50-100% speed increases.
                 */
                index += (index < size - 1);
            }
        }
        return index;
    };
    static constexpr uint32_t SIMPLE_IDX_START = 0;
    static constexpr uint32_t SIMPLE_IDX_END   = SIMPLE_IDX_START + TONE_COUNT_SIMPLE;
    static constexpr uint32_t LS_IDX_START     = SIMPLE_IDX_END;
    static constexpr uint32_t LS_IDX_END       = LS_IDX_START + TONE_COUNT_LS;
    static constexpr uint32_t MIDI_IDX_START   = LS_IDX_END;
    static constexpr uint32_t MIDI_IDX_END     = MIDI_IDX_START + TONE_COUNT_MIDI;
private:
    uint32_t minOntimeUS    = 0;
    float maxOntimeUS    = 10;
    float maxDuty        = 0.01f;
    float signalDuty     = 0.0f;
    uint32_t activeTones = 0;
    Tone tonelist[TONE_COUNT_TOTAL];
};

#endif /* TONELIST_H_ */
