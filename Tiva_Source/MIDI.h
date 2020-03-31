/*
 * MIDI.h
 *
 *  Created on: 27.03.2020
 *      Author: Max
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
#include "System.h"


class MIDI
{
public:
    MIDI();
    virtual ~MIDI();
    void init(System* sys, uint32_t uartNum, uint32_t baudRate, void (*ISR)(void));
    void enable();
    void disable();
    void play();
    void stop();
    void uartISR();
    void setVolumeMax(float maxVol);
    void setVolumeMode(uint32_t volMode);
    bool isEnabled();
    void updateFrequencyOntime();
    float getOntimeUS();
    float getFrequency();
    void setADSR(bool enable);
    static constexpr uint32_t MIDI_VOL_RELATIVE = 0;
    static constexpr uint32_t MIDI_VOL_ABSOLUTE = 1;

private:
    void resetAllValues();
    void updateADSR();
    float getLFOVal();
    static constexpr uint32_t MIDI_UART_SYSCTL_PERIPH      = 0;
    static constexpr uint32_t MIDI_UART_BASE               = 1;
    static constexpr uint32_t MIDI_UART_PORT_SYSCTL_PERIPH = 2;
    static constexpr uint32_t MIDI_UART_PORT_BASE          = 3;
    static constexpr uint32_t MIDI_UART_RX_PIN_CFG         = 4;
    static constexpr uint32_t MIDI_UART_TX_PIN_CFG         = 5;
    static constexpr uint32_t MIDI_UART_RX_PIN             = 6;
    static constexpr uint32_t MIDI_UART_TX_PIN             = 7;
    static constexpr uint32_t MIDI_UART_INT                = 8;
    const uint32_t MIDI_UART_MAPPING[5][9] = {{SYSCTL_PERIPH_UART0, UART0_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART0},
                                              /*TBD*/{SYSCTL_PERIPH_UART1, UART1_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART1},
                                              /*TBD*/{SYSCTL_PERIPH_UART2, UART2_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART2},
                                              {SYSCTL_PERIPH_UART3, UART3_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA4_U3RX, GPIO_PA5_U3TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART3},
                                              {SYSCTL_PERIPH_UART4, UART4_BASE, SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, GPIO_PK0_U4RX, GPIO_PK1_U4TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART4}};

    static constexpr uint32_t MIDI_ADSR_ATTACK_AMP     = 0;
    static constexpr uint32_t MIDI_ADSR_ATTACK_DUR_US  = 1;
    static constexpr uint32_t MIDI_ADSR_DECAY_AMP      = 2;
    static constexpr uint32_t MIDI_ADSR_DECAY_DUR_US   = 3;
    static constexpr uint32_t MIDI_ADSR_SUSTAIN_AMP    = 4;
    static constexpr uint32_t MIDI_ADSR_SUSTAIN_DUR_US = 5;
    static constexpr uint32_t MIDI_ADSR_RELEASE_AMP    = 6;
    static constexpr uint32_t MIDI_ADSR_RELEASE_DUR_US = 7;
    static constexpr uint32_t MIDI_ADSR_PROGRAM_COUNT  = 6;
    // Note Durations cant be 0. To "skip" D/S/R set Duration to 1.0f (any very small value) and Amplitude to exactly the previous one.
    // 0: Normal ("Piano")
    // 1: Slow Pad (Slooow rise, sloow fall)
    // 2: Slow Step Pad (As Slow Pad, but with a faster step at the beginning. good for faster notes
    // 3: Pad
    // 4: Staccato (no long notes possible. They're short. Always.
    // 5: Legator (release = prolonged sustain)
    //                                                             Attack Amp/Dur        Decay Amp/Dur         Sustain Amp/Dur        Release Amp/Dur
    const float MIDI_ADSR_PROGRAMS[MIDI_ADSR_PROGRAM_COUNT][9] = {{1.0f,    7000.0f,     0.5f,   15000.0f,     0.25f,  3000000.0f,     0.0f,    3000.0f},
                                                                  {1.0f, 4000000.0f,     1.0f,       1.0f,     1.00f,        1.0f,     0.0f, 2000000.0f},
                                                                  {0.3f,    8000.0f,     1.0f, 4000000.0f,     1.00f,        1.0f,     0.0f, 2000000.0f},
                                                                  {1.0f, 1500000.0f,     1.0f,       1.0f,     1.00f,        1.0f,     0.0f,  750000.0f},
                                                                  {1.0f,   10000.0f,     0.2f,   35000.0f,     0.00f,    10000.0f,     0.0f,   10000.0f},
                                                                  {1.0f,    7000.0f,     0.5f,   10000.0f,     0.25f,  3000000.0f,     0.0f, 3000000.0f},
    };
    System* midiSys;
    bool midiEnabled = false;

    // Values updated inside MIDI ISR
    volatile bool midiISRADSREnabled        = false;
    volatile bool midiISRNewData            = true;
    volatile bool midiISRSustainPedal       = false;
    volatile bool midiISRDamperPedal        = false;
    volatile bool midiISRNoteNew            = false;
    volatile int32_t midiISRPitchBend       = 0;
    volatile uint32_t midiISRAfterTouch     = 0;
    volatile uint32_t midiISRChnVol         = 127;
    volatile uint32_t midiISRDataIndex      = 0;
    volatile uint32_t midiISRData[3]        = {0, 0, 0};
    volatile uint32_t midiISRModWheel       = 0;
    volatile uint32_t midiISRNoteAfterTouch = 0;
    volatile uint32_t midiISRNoteNum        = 0;
    volatile uint32_t midiISRNoteVel        = 0;
    volatile uint32_t midiISRProgram        = 0;

    bool midiADSREnabled  = false;
    bool midiADSRNextMode = false;
    float midiADSROntimeUS = 0.0f;
    float midiADSRResolutionUS = 10000.0f;
    float midiADSRTimeUS = 0.0f;
    uint32_t midiNoteVelPedal      = 0;
    uint32_t midiADSRMode = 'R';
    volatile bool midiPlaying = false;
    uint32_t midiVolMode = MIDI_VOL_RELATIVE;
    float midiMaxVol = 0.0f;
    uint32_t midiUARTNum;
    volatile uint32_t midiUARTBase;
    uint32_t midiChnVol = 127;
    uint32_t midiNoteNum           = 0;
    uint32_t midiNoteVel           = 0;
    uint32_t midiNoteAfterTouch    = 0;
    uint32_t midiAfterTouch        = 0;
    uint32_t midiModWheel          = 0;
    int32_t  midiPitchBend         = 0;
    uint32_t midiProgram           = 0;
    bool midiSustainPedal          = false;
    float midiFrequency                     = 0.0f;
    float midiOntimeUS             = 0.0f;
    float midiLFOPeriodUS          = 200000.0f;
};

#endif /* MIDI_H_ */
