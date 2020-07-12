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
#include "inc/hw_memmap.h"              // Macros defining the memory map of the Tiva C Series device. This includes defines such as peripheral base address locations such as GPIO_PORTF_BASE.
#include "inc/hw_types.h"               // Defines common types and macros.
#include "inc/hw_gpio.h"                // Defines and Macros for GPIO hardware.
#include "inc/hw_uart.h"                // Defines and macros used when accessing the UART.
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"          // Mapping of peripherals to pins for all parts.
#include "driverlib/sysctl.h"           // Defines and macros for System Control API of DriverLib. This includes API functions such as SysCtlClockSet.
#include "driverlib/gpio.h"             // Defines and macros for GPIO API of DriverLib. This includes API functions such as GPIOPinWrite.
#include "driverlib/interrupt.h"        // Defines and macros for NVIC Controller (Interrupt) API of driverLib. This includes API functions such as IntEnable and IntPrioritySet.
#include "driverlib/uart.h"             // Defines and macros for UART API of driverLib.
#include "InterrupterConfig.h"
#include "System.h"
#include "Note.h"
#include "Channel.h"


class MIDI
{
public:
    MIDI();
    virtual ~MIDI();
    void init(System* sys, uint32_t usbUartNum, uint32_t baudRate, void (*usbISR)(void), uint32_t midiUartNum, void (*MIDIISR)(void));
    void usbUartISR();
    void midiUartISR();
    void addData(uint8_t data);
    void UARTEnable();
    void UARTDisable();
    void start();
    void stop();
    void newData(uint32_t c);
    void setVolSettings(uint32_t coil, float ontimeUSMax, float dutyMax, uint32_t volMode);
    void setChannels(uint32_t coil, uint32_t chns);
    void setPan(uint32_t coil, uint32_t pan);
    void setTotalMaxDutyPerm(uint32_t coil, float maxDuty);
    bool isPlaying();
    void process();
    void setADSR(bool enable);
    uint32_t activeNotes[COIL_COUNT];
    Note *orderedNotes[COIL_COUNT][MAX_VOICES];

private:
    void updateEffects();
    float getLFOVal(uint32_t channel);
    void removeDeadNotes();
    void resetNote(uint32_t coil, uint32_t note);
    static constexpr uint32_t MIDI_UART_SYSCTL_PERIPH      = 0;
    static constexpr uint32_t MIDI_UART_BASE               = 1;
    static constexpr uint32_t MIDI_UART_PORT_SYSCTL_PERIPH = 2;
    static constexpr uint32_t MIDI_UART_PORT_BASE          = 3;
    static constexpr uint32_t MIDI_UART_RX_PIN_CFG         = 4;
    static constexpr uint32_t MIDI_UART_TX_PIN_CFG         = 5;
    static constexpr uint32_t MIDI_UART_RX_PIN             = 6;
    static constexpr uint32_t MIDI_UART_TX_PIN             = 7;
    static constexpr uint32_t MIDI_UART_INT                = 8;
    const uint32_t MIDI_UART_MAPPING[8][9] = {{SYSCTL_PERIPH_UART0, UART0_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART0},
                                              {SYSCTL_PERIPH_UART1, UART1_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTB_BASE, GPIO_PB0_U1RX, GPIO_PB1_U1TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART1},
                                              {SYSCTL_PERIPH_UART2, UART2_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA6_U2RX, GPIO_PA7_U2TX, GPIO_PIN_6, GPIO_PIN_7, INT_UART2},
                                              {SYSCTL_PERIPH_UART3, UART3_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA4_U3RX, GPIO_PA5_U3TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART3},
                                              {SYSCTL_PERIPH_UART4, UART4_BASE, SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, GPIO_PK0_U4RX, GPIO_PK1_U4TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART4},
                                              {SYSCTL_PERIPH_UART5, UART5_BASE, SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PC6_U5RX, GPIO_PC7_U5TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART5},
                                              {SYSCTL_PERIPH_UART6, UART6_BASE, SYSCTL_PERIPH_GPIOP, GPIO_PORTP_BASE, GPIO_PP0_U6RX, GPIO_PP1_U6TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART6},
                                              {SYSCTL_PERIPH_UART7, UART7_BASE, SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PC4_U7RX, GPIO_PC5_U7TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART7}};

    static constexpr uint32_t MIDI_ADSR_MODE_ATTACK  = 0;
    static constexpr uint32_t MIDI_ADSR_MODE_DECAY   = 1;
    static constexpr uint32_t MIDI_ADSR_MODE_SUSTAIN = 2;
    static constexpr uint32_t MIDI_ADSR_MODE_RELEASE = 3;

    static constexpr uint32_t MIDI_ADSR_ATTACK_AMP        = 0;
    static constexpr uint32_t MIDI_ADSR_ATTACK_INVDUR_US  = 1;
    static constexpr uint32_t MIDI_ADSR_DECAY_AMP         = 2;
    static constexpr uint32_t MIDI_ADSR_DECAY_INVDUR_US   = 3;
    static constexpr uint32_t MIDI_ADSR_SUSTAIN_AMP       = 4;
    static constexpr uint32_t MIDI_ADSR_SUSTAIN_INVDUR_US = 5;
    static constexpr uint32_t MIDI_ADSR_RELEASE_AMP       = 6;
    static constexpr uint32_t MIDI_ADSR_RELEASE_INVDUR_US = 7;
    static constexpr uint32_t MIDI_ADSR_PROGRAM_COUNT     = 9;
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
    const float MIDI_ADSR_PROGRAMS[MIDI_ADSR_PROGRAM_COUNT + 1][9] = {{1.0f,              1.0f,     1.0f,              1.0f,     1.0f,               1.0f,      0.0f,              1.0f},

                                                                      {1.0f, 1.0f /   30000.0f,     0.5f, 1.0f /     250.0f,     0.10f, 1.0f /  3500000.0f,     0.0f, 1.0f /     150.0f},
                                                                      {1.0f, 1.0f / 4000000.0f,     1.0f, 1.0f /       1.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f / 2000000.0f},
                                                                      {0.3f, 1.0f /    8000.0f,     1.0f, 1.0f / 4000000.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f / 2000000.0f},
                                                                      {1.0f, 1.0f / 1500000.0f,     1.0f, 1.0f /       1.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f /  750000.0f},
                                                                      {1.0f, 1.0f /    3000.0f,     0.3f, 1.0f /   30000.0f,     0.00f, 1.0f /   400000.0f,     0.0f, 1.0f /  400000.0f},
                                                                      {1.0f, 1.0f /    7000.0f,     0.5f, 1.0f /   10000.0f,     0.25f, 1.0f /  3000000.0f,     0.0f, 1.0f / 3000000.0f},
                                                                      {0.3f, 1.0f /    8000.0f,     1.0f, 1.0f / 4000000.0f,     1.00f, 1.0f /        1.0f,     0.0f, 1.0f /  400000.0f},
                                                                      {2.0f, 1.0f /   30000.0f,     1.0f, 1.0f /    2500.0f,     0.20f, 1.0f /  3500000.0f,     0.0f, 1.0f /   10000.0f},
                                                                      {3.0f, 1.0f /    3000.0f,     1.0f, 1.0f /   27000.0f,     0.00f, 1.0f /   400000.0f,     0.0f, 1.0f /  400000.0f},
    };
    static constexpr uint32_t midiEffectResolutionUS = 1000;

    System* midiSys;
    Channel channels[16];
    Note notes[COIL_COUNT][MAX_VOICES];
    float midiAbsFreq[COIL_COUNT];
    float midiSingleNoteMaxDuty[COIL_COUNT];
    float midiSingleNoteMaxOntimeUS[COIL_COUNT];
    float midiTotalMaxDutyUS[COIL_COUNT];
    float midiCoilPan[COIL_COUNT];
    uint8_t midiVolMode[COIL_COUNT];
    static constexpr uint32_t midiUARTBufferSize = 1024;
    volatile uint8_t midiUARTBuffer[midiUARTBufferSize];
    volatile uint32_t midiUARTBufferWriteIndex = 0;
    volatile uint32_t midiUARTBufferReadIndex = 0;

    bool midiNoteChange            = true;
    uint32_t midiDataIndex      = 0;
    uint32_t midiData[3]        = {0, 0, 0};
    uint32_t midiChannel = 0;
    uint32_t midiRPN = 127;

    bool midiADSREnabled  = false;
    float midiADSRTimeUS = 0.0f;
    bool midiPlaying = false;
    uint32_t midiUSBUARTNum, midiMIDIUARTNum;
    uint32_t midiUSBUARTBase, midiMIDIUARTBase;
    float midiLFOPeriodUS          = 200000.0f;
};

#endif /* MIDI_H_ */
