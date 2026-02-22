/*
 * ToneList.cpp
 *
 *  Created on: 21.02.2026
 *      Author: Max Zuidberg
 */

#include <ToneList.h>

ToneList::ToneList()
{
    // Auto-generated constructor stub
}

ToneList::~ToneList()
{
    // Auto-generated destructor stub
}

void ToneList::setMinOntimeUS(float minOntimeUS)
{
    this->minOntimeUS = minOntimeUS;
    for (uint32_t toneNum = 0; toneNum < TONE_COUNT_TOTAL; toneNum++)
    {
        tonelist[toneNum].minDuty = minOntimeUS / tonelist[toneNum].periodUS;
    }
    limit();
};


void ToneList::setMaxOntimeUS(float maxOntimeUS)
{
    this->maxOntimeUS = maxOntimeUS;
    for (uint32_t toneNum = 0; toneNum < TONE_COUNT_TOTAL; toneNum++)
    {
        tonelist[toneNum].ontimeUS = Branchless::min(tonelist[toneNum].ontimeUS, (uint32_t)maxOntimeUS);
        tonelist[toneNum].duty = (float) tonelist[toneNum].ontimeUS / tonelist[toneNum].periodUS;
    }
    limit();
};


template<ToneList::Owner owner> void ToneList::updateTone(uint32_t index, Tone::Type type, uint32_t ontimeUS, uint32_t periodUS, uint32_t lowerFreq, uint32_t upperFreq)
{
    // Make sure each owner stays within their region
    constexpr uint32_t START =
        owner == Owner::SIMPLE     ? SIMPLE_IDX_START :
        owner == Owner::LIGHTSABER ? LS_IDX_START :
        owner == Owner::MIDI_LIVE  ? MIDI_IDX_START :
                                     0;
    constexpr uint32_t END =
        owner == Owner::SIMPLE     ? SIMPLE_IDX_END :
        owner == Owner::LIGHTSABER ? LS_IDX_END :
        owner == Owner::MIDI_LIVE  ? MIDI_IDX_END :
                                     0;        

    index += START;
    if (index >= END)
    {
        return;
    }

    auto& tone = tonelist[index];

    bool newDuty = false;
    ontimeUS = Branchless::min(ontimeUS, (uint32_t)maxOntimeUS);
    if (ontimeUS != tone.ontimeUS)
    {
        newDuty = true;
        tone.ontimeUS = ontimeUS;
        tone.limitedOntimeUS = ontimeUS;

        // Adjust number of active tones if required
        if (ontimeUS && !tone.ontimeUS)
        {
            activeTones++;
        }
        else if (!ontimeUS && tone.ontimeUS)
        {
            activeTones--;
        }
    }
    if (tone.type != type)
    {
        newDuty = true;
        tone.type = type;
    }
    if (lowerFreq != tone.lowerFreq || upperFreq != tone.upperFreq)
    {
        if (lowerFreq > 0 && upperFreq > lowerFreq)
        {
            /*
            * Time averaged duty cycle of noise with distribution p(f) 
            * from f_low to f_high with constant ontime:
            * d = ontime / int(p(f) / f df, f_low..f_high)
            * 
            * With p(f) = const = 1 / (f_high - f_low):
            * d = ontime / int(1 / (f_high - f_low) / f df, f_low..f_high)
            * d = ontime * (f_high - f_low) / int(1 / f df, f_low..f_high)
            * d = ontime * (f_high - f_low) / (log(f_high) - log(f_low))
            * d = ontime * (f_high - f_low) / log(f_high / f_low))
            * => period = ontime / d = log(...) / (f_high - f_low)
            */
            newDuty = true;
            tone.lowerFreq = lowerFreq;
            tone.upperFreq = upperFreq;
            periodUS  = 1e6f / (upperFreq - lowerFreq) * logf((float) upperFreq / lowerFreq);
        }
    }
    if (periodUS && periodUS != tone.periodUS)
    {
        newDuty = true;
        tone.nextFireUS -= tone.periodUS;
        tone.nextFireUS += periodUS;
        tone.periodUS = periodUS;
    }
    if (newDuty)
    {
        tone.duty    = float(ontimeUS)    / float(periodUS);
        tone.minDuty = float(minOntimeUS) / float(periodUS);
        limit();
    }
}
// cpp must explicitly include all template instantiations
// else the implementation must be included in the header file
// (either directly or through an included tpp implementation file).
template void ToneList::updateTone<ToneList::Owner::SIMPLE>(uint32_t, Tone::Type, uint32_t, uint32_t, uint32_t, uint32_t);
template void ToneList::updateTone<ToneList::Owner::LIGHTSABER>(uint32_t, Tone::Type, uint32_t, uint32_t, uint32_t, uint32_t);
template void ToneList::updateTone<ToneList::Owner::MIDI_LIVE>(uint32_t, Tone::Type, uint32_t, uint32_t, uint32_t, uint32_t);

template<ToneList::Owner owner> uint32_t ToneList::getFreeTone()
{
    constexpr uint32_t START =
        owner == Owner::SIMPLE     ? SIMPLE_IDX_START :
        owner == Owner::LIGHTSABER ? LS_IDX_START :
        owner == Owner::MIDI_LIVE  ? MIDI_IDX_START :
                                     0;
    constexpr uint32_t END =
        owner == Owner::SIMPLE     ? SIMPLE_IDX_END :
        owner == Owner::LIGHTSABER ? LS_IDX_END :
        owner == Owner::MIDI_LIVE  ? MIDI_IDX_END :
                                     0;        
    #pragma UNROLL(MIDI_IDX_END)
    for (uint32_t i = START; i < END; i++)
    {
        if (!tonelist[i].ontimeUS)
        {
            return i - START;
        }
    }
    return END;
};
// cpp must explicitly include all template instantiations
// else the implementation must be included in the header file
// (either directly or through an included tpp implementation file).
template uint32_t ToneList::getFreeTone<ToneList::Owner::SIMPLE>();
template uint32_t ToneList::getFreeTone<ToneList::Owner::LIGHTSABER>();
template uint32_t ToneList::getFreeTone<ToneList::Owner::MIDI_LIVE>();


void ToneList::limit()
{
    float unlimitedDuty = 0.0f;
    float minimumDuty   = 0.0f;
    float ontimeOffsetUS = minOntimeUS;
    for (uint32_t toneNum = 0; toneNum < TONE_COUNT_TOTAL; toneNum++)
    {
        unlimitedDuty += tonelist[toneNum].duty;
        minimumDuty   += tonelist[toneNum].periodUS;
    }
    // Don't add minOntime if that part alone already exceeds the duty limit.
    // A more granular approach would be desirable but is also more complicated 
    // unfortunately.
    if (minimumDuty >= maxDuty)
    {
        ontimeOffsetUS = 0.0f;
        minimumDuty = 0.0f;
    }
    float dutyBudget = maxDuty - minimumDuty;

    // Signal duty cannot and must not exceed the limit (maxDuty). 
    signalDuty = Branchless::min(dutyBudget, unlimitedDuty);

    // Since signalDuty != unlimitedDuty is only true if unlimitedDuty exceeds maxDuty
    // this only equals to <1.0 if unlimitedDuty exceeds maxDuty. 
    // There used to be checks here and there but it seems faster and cleaner to just 
    // always apply this factor. 
    // Note: you have to apply it (at least once) even if it's equal to 1.0 to "unapply"  
    // a previous limit.
    unlimitedDuty = signalDuty / unlimitedDuty;
    for (uint32_t toneNum = 0; toneNum < TONE_COUNT_TOTAL; toneNum++)
    {
        tonelist[toneNum].limitedOntimeUS = ontimeOffsetUS + tonelist[toneNum].ontimeUS * unlimitedDuty;
    }
}

void ToneList::applyTimeOffsetUS(uint32_t offsetUS)
{
    for (uint32_t toneNum = 0; toneNum < TONE_COUNT_TOTAL; toneNum++)
    {
        /*
         * Branchless version of
         * if (tonelist[toneNum].nextFireUS > offsetUS)
         * {
         *     tonelist[toneNum].nextFireUS -= offsetUS;
         * }
         */
         tonelist[toneNum].nextFireUS -= offsetUS * (tonelist[toneNum].nextFireUS > offsetUS);
    }
}
