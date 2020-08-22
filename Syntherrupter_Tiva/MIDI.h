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
#include <math.h>
#include "InterrupterConfig.h"
#include "System.h"
#include "Channel.h"
#include "UART.h"
#include "ByteBuffer.h"
#include "Note.h"
#include "ToneList.h"


extern System sys;


class MIDI
{
public:
    MIDI();
    virtual ~MIDI();
    static void init(uint32_t usbBaudRate, void (*usbISR)(void), uint32_t midiUartPort, uint32_t midiUartRx, uint32_t midiUartTx, void (*midiISR)(void));
    void updateToneList();
    void setCoilsToneList(ToneList* tonelist);
    void setCoilNum(uint32_t num);
    static void start();
    static void stop();
    static void resetNRPs(uint32_t chns = 0xffff);
    void setVolSettings(float ontimeUSMax, float dutyMax);
    void setChannels(uint32_t chns);
    void setPan(uint32_t pan);
    void setPanReach(uint32_t reach);
    void setMaxVoices(uint32_t maxVoices);
    static bool isPlaying();
    static void process(bool force = false);
    static Channel channels[16];
    static UART usbUart, midiUart;
    static ByteBuffer otherBuffer;

private:
    static bool processBuffer(uint32_t b);
    static void updateEffects();
    void setPanVol(Note* note);
    static float getLFOVal(uint32_t channel);
    static void removeDeadNotes();
    static constexpr uint32_t UART_SYSCTL_PERIPH      = 0;
    static constexpr uint32_t UART_BASE               = 1;
    static constexpr uint32_t UART_PORT_SYSCTL_PERIPH = 2;
    static constexpr uint32_t UART_PORT_BASE          = 3;
    static constexpr uint32_t UART_RX_PIN_CFG         = 4;
    static constexpr uint32_t UART_TX_PIN_CFG         = 5;
    static constexpr uint32_t UART_RX_PIN             = 6;
    static constexpr uint32_t UART_TX_PIN             = 7;
    static constexpr uint32_t UART_INT                = 8;
    const uint32_t UART_MAPPING[8][9] = {{SYSCTL_PERIPH_UART0, UART0_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART0},
                                              {SYSCTL_PERIPH_UART1, UART1_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTB_BASE, GPIO_PB0_U1RX, GPIO_PB1_U1TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART1},
                                              {SYSCTL_PERIPH_UART2, UART2_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA6_U2RX, GPIO_PA7_U2TX, GPIO_PIN_6, GPIO_PIN_7, INT_UART2},
                                              {SYSCTL_PERIPH_UART3, UART3_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA4_U3RX, GPIO_PA5_U3TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART3},
                                              {SYSCTL_PERIPH_UART4, UART4_BASE, SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, GPIO_PK0_U4RX, GPIO_PK1_U4TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART4},
                                              {SYSCTL_PERIPH_UART5, UART5_BASE, SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PC6_U5RX, GPIO_PC7_U5TX, GPIO_PIN_6, GPIO_PIN_7, INT_UART5},
                                              {SYSCTL_PERIPH_UART6, UART6_BASE, SYSCTL_PERIPH_GPIOP, GPIO_PORTP_BASE, GPIO_PP0_U6RX, GPIO_PP1_U6TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART6},
                                              {SYSCTL_PERIPH_UART7, UART7_BASE, SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PC4_U7RX, GPIO_PC5_U7TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART7}};

    static constexpr uint32_t ADSR_MODE_ATTACK  = 0;
    static constexpr uint32_t ADSR_MODE_DECAY   = 1;
    static constexpr uint32_t ADSR_MODE_SUSTAIN = 2;
    static constexpr uint32_t ADSR_MODE_RELEASE = 3;

    static constexpr uint32_t ADSR_ATTACK_AMP        = 0;
    static constexpr uint32_t ADSR_ATTACK_INVDUR_US  = 1;
    static constexpr uint32_t ADSR_DECAY_AMP         = 2;
    static constexpr uint32_t ADSR_DECAY_INVDUR_US   = 3;
    static constexpr uint32_t ADSR_SUSTAIN_AMP       = 4;
    static constexpr uint32_t ADSR_SUSTAIN_INVDUR_US = 5;
    static constexpr uint32_t ADSR_RELEASE_AMP       = 6;
    static constexpr uint32_t ADSR_RELEASE_INVDUR_US = 7;
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
    //                                                                 Attack Amp/Invers Dur.       Decay Amp/Invers Dur.        Sustain Amp/Invers Dur.       Release Amp/Invers Dur.
    static constexpr float ADSR_PROGRAMS[ADSR_PROGRAM_COUNT + 1][9] = {{1.0f,              1.0f,     1.0f,              1.0f,     1.0f,               1.0f,      0.0f,              1.0f},

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
    static constexpr uint32_t effectResolutionUS = 1000;

    static constexpr uint32_t    bufferCount = 3;
    static constexpr ByteBuffer* bufferList[bufferCount] = {&(usbUart.buffer), &(midiUart.buffer), &otherBuffer};;
    static uint8_t               bufferMidiStatus[bufferCount];
    static constexpr uint32_t totalNotesLimit = 64;
    static Note               notes[totalNotesLimit];
    static Note*              orderedNotes[totalNotesLimit];
    static uint32_t           notesCount;
    ToneList* tonelist;
    float absFreq               =  0.0f;
    float singleNoteMaxDuty     =  0.0f;
    float singleNoteMaxOntimeUS =  0.0f;
    float coilPan               = -1.0f;
    float inversPanReach        =  0.0f;
    uint8_t volMode             =  3;
    uint16_t activeChannels     =  0xffff;
    uint32_t coilMaxVoices      =  0;
    uint32_t rawOntime          =  0;
    uint32_t coilNum            =  0;
    uint8_t  coilBit            =  0;
    bool coilChange             = false;
    bool coilPanChanged         = false;
    bool panConstVol            = false;

    static float ADSRTimeUS;
    static bool playing;
    static constexpr float LFOPeriodUS          = 200000.0f;
};

#endif /* H_ */
