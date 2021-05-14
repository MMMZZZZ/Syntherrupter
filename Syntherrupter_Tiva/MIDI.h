/*
 * MIDI.h
 *
 *  Created on: 27.03.2020
 *      Author: Max Zuidberg
 */

#ifndef MIDI_H_
#define MIDI_H_


#include <stdbool.h>
#include <stdint.h>
#include "InterrupterConfig.h"
#include "System.h"
#include "Channel.h"
#include "UART.h"
#include "ByteBuffer.h"
#include "ToneList.h"
#include "NoteList.h"
#include "MIDIProgram.h"
#include "Sysex.h"


class MIDI
{
public:
    MIDI();
    virtual ~MIDI();
    void updateToneList();
    void setCoilsToneList(ToneList* tonelist)
    {
        this->tonelist = tonelist;
    };
    void setCoilNum(uint32_t num);
    void setVolSettingsProm(float ontimeUSMax, float dutyMaxProm);
    void setVolSettings(float ontimeUSMax, float dutyMax);
    void setOntimeUS(float ontimeUSMax)
    {
        setVolSettings(ontimeUSMax, singleNoteMaxDuty);
    };
    void setDuty(float dutyMax)
    {
        setVolSettings(singleNoteMaxOntimeUS, dutyMax);
    };
    void setChannels(uint32_t chns);
    void setPan(float pan);
    void setPanReach(float reach);
    void setPanConstVol(bool cnst)
    {
        panConstVol    = cnst;
        coilPanChanged = true;
    };
    void setMaxVoices(uint32_t maxVoices);
    float getOntimeUS()
    {
        return singleNoteMaxOntimeUS;
    };
    float getDuty()
    {
        return singleNoteMaxDuty;
    };
    static void init(uint32_t usbBaudRate, void (*usbISR)(void),
                     uint32_t midiUartPort, uint32_t midiUartRx,
                     uint32_t midiUartTx, void (*midiISR)(void));
    static void start();
    static void stop();
    static bool isPlaying()
    {
        return playing;
    };
    static void resetNRPs(uint32_t chns = 0xffff);
    static void process();
    static SysexMsg getSysex()
    {
        if (sysexMsg.newMsg)
        {
            sysexMsg.newMsg--;
        }
        return sysexMsg;
    }
    static Channel channels[16];
    static UART usbUart, midiUart;
    static ByteBuffer otherBuffer;
    static constexpr uint32_t MAX_PROGRAMS = 64;
    static MIDIProgram programs[MAX_PROGRAMS];

private:
    static bool processBuffer(uint32_t b);
    static void updateEffects(Note* note);
    static void processSysex(uint32_t sysexNum, uint32_t targetLSB, uint32_t targetMSB, int32_t sysexVal);
    void setPanVol(Note* note)
    {
        if (note->panChanged || coilPanChanged)
        {
            if (note->panChanged & coilBit)
            {
                note->panChanged &= ~coilBit;
            }
            // 1.01f instead of 1.0f to include the borders of the range.
            note->panVol[coilNum] = 1.01f - inversPanReach * fabsf(note->pan - coilPan);
            if (note->panVol[coilNum] <= 0.0f)
            {
                note->panVol[coilNum] = 0.0f;
            }
            else if (panConstVol)
            {
                note->panVol[coilNum] = 1.0f;
            }

            if (coilPan < 0.0f || note->channel->notePanMode == Channel::NOTE_PAN_OMNI)
            {
                note->panVol[coilNum] = 1.0f;
            }
        }
    };
    static float getLFOVal(Channel* channel)
    {
        if (channel->modulation)
        {
            /*
             *               /            t   \
             * LFO_SINE = sin| 2 * Pi * ----- |
             *               \           T_0  /
             *
             *       1   /  LFO_SINE + 1       ModWheelValue    \
             * val = - * | --------------- * ------------------ |
             *       2   \        2           MaxModWheelValue  /
             *
             * sine wave between 0 and 1 mapped to the desired modulation depth (50% max).
             */
            return (sinf(6.283185307179586f * float(System::getSystemTimeUS()) / LFO_PERIOD_US) + 1) / 4.0f
                    * channel->modulation;
        }
        else
        {
            return 0.0f;
        }
    };
    static void removeDeadNotes();
    static float getFreq(float pitch)
    {
        if (pitch <= 0.0f)
        {
            return freqTable[0];
        }
        else if (pitch >= 127.0f)
        {
            return freqTable[127];
        }
        else
        {
            uint32_t intPitch = pitch;
            float fracPitch = pitch - intPitch;
            float freq = freqTable[intPitch];
            freq += (freqTable[intPitch + 1] - freq) * fracPitch;
            return freq;
        }
    };

    static float freqTable[128];

    static constexpr uint32_t ADSR_PROGRAM_COUNT     = 9;
    // TODO: The following list is missing the newer sounds.
    // Note Durations cant be 0. To "skip" D/S/R set Duration to 1.0f (any very small value) and Amplitude to exactly the previous one.
    // 0: No ADSR
    // 1: Normal ("Piano")
    // 2: Slow Pad (Slooow rise, sloow fall)
    // 3: Slow Step Pad (As Slow Pad, but with a faster step at the beginning. good for faster notes
    // 4: Pad
    // 5: Staccato (no long notes possible. They're short. Always.
    // 6: Legator (release = prolonged sustain)
    static constexpr float ADSR_LEGACY_PROGRAMS[ADSR_PROGRAM_COUNT + 1][8]

             // Attack Amp/Invers Dur.       Decay Amp/Invers Dur.        Sustain Amp/Invers Dur.       Release Amp/Invers Dur.
           = {{1.0f,              1.0f,     1.0f,              1.0f,     1.0f,               1.0f,      0.0f,              1.0f},

              {1.0f, 1.0f /   30000.0f,     0.5f, 1.0f /   10000.0f,     0.10f, 1.0f /  3500000.0f,     0.0f, 1.0f /   10000.0f},
              {1.0f, 1.0f / 4000000.0f,     1.0f, 1.0f /       1.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f / 1000000.0f},
              {0.3f, 1.0f /    8000.0f,     1.0f, 1.0f / 4000000.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f / 1000000.0f},
              {1.0f, 1.0f / 1500000.0f,     1.0f, 1.0f /       1.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f /  500000.0f},
              {1.0f, 1.0f /    3000.0f,     0.4f, 1.0f /   30000.0f,     0.00f, 1.0f /   400000.0f,     0.0f, 1.0f /       1.0f},
              {1.0f, 1.0f /    7000.0f,     0.5f, 1.0f /   10000.0f,     0.25f, 1.0f /  3000000.0f,     0.0f, 1.0f / 3000000.0f},
              {0.3f, 1.0f /    8000.0f,     1.0f, 1.0f / 4000000.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f /  400000.0f},
              {2.0f, 1.0f /   30000.0f,     1.0f, 1.0f /    2500.0f,     0.10f, 1.0f /  3500000.0f,     0.0f, 1.0f /   10000.0f},
              {3.0f, 1.0f /    3000.0f,     1.0f, 1.0f /   27000.0f,     0.00f, 1.0f /   400000.0f,     0.0f, 1.0f /       1.0f},
    };

    static constexpr uint32_t effectResolutionUS = 2000;

    static constexpr uint32_t SYSEX_MAX_SIZE = 16;
    static constexpr uint32_t SYSEX_PROTOCOL_VERSION = 1;
    static constexpr uint32_t    BUFFER_COUNT = 3;
    static constexpr ByteBuffer* BUFFER_LIST[BUFFER_COUNT] = {&(usbUart.buffer), &(midiUart.buffer), &otherBuffer};;
    static constexpr uint32_t MAX_NOTES_COUNT = 64;
    static NoteList notelist;
    static uint32_t notesCount;
    static uint32_t sysexDeviceID;
    static SysexMsg sysexMsg;
    ToneList* tonelist;
    float absFreq               =  0.0f;
    float singleNoteMaxDuty     =  0.0f;
    float singleNoteMaxOntimeUS =  0.0f;
    float coilPan               = -1.0f;
    float inversPanReach        =  0.0f;
    uint8_t volMode             =  3;
    uint16_t activeChannels     =  0xffff;
    uint32_t coilMaxVoices      =  0;
    uint32_t coilNum            =  0;
    uint8_t  coilBit            =  0;
    bool coilChange             = false;
    bool coilPanChanged         = false;
    bool panConstVol            = false;

    static bool playing;
    static constexpr float LFO_PERIOD_US          = 200000.0f;
};

#endif /* H_ */
