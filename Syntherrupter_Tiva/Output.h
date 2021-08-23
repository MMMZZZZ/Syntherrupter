/*
 * Output.h
 *
 *  Created on: 02.06.2021
 *      Author: Max Zuidberg
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_


#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "inc/hw_memmap.h"              // Macros defining the memory map of the Tiva C Series device. This includes defines such as peripheral base address locations such as GPIO_PORTF_BASE.
#include "inc/hw_types.h"               // Defines common types and macros.
#include "inc/hw_gpio.h"                // Defines and Macros for GPIO hardware.
#include "inc/hw_timer.h"               // Defines and macros used when accessing the timer.
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"          // Mapping of peripherals to pins for all parts.
#include "driverlib/sysctl.h"           // Defines and macros for System Control API of DriverLib. This includes API functions such as SysCtlClockSet.
#include "driverlib/gpio.h"             // Defines and macros for GPIO API of DriverLib. This includes API functions such as GPIOPinWrite.
#include "driverlib/interrupt.h"        // Defines and macros for NVIC Controller (Interrupt) API of driverLib. This includes API functions such as IntEnable and IntPrioritySet.
#include "driverlib/timer.h"            // Defines and macros for timer API of driverLib.
#include "InterrupterConfig.h"
#include "Branchless.h"
#include "System.h"
#include "Pulse.h"


class Output
{
public:
    Output();
    virtual ~Output();
    void init(uint32_t timer, void (*ISR)(void));
    void addPulse(Pulse& pulse);
    void setMaxOntimeUS(uint32_t maxOntimeUS);
    void setMinOfftimeUS(uint32_t minOfftimeUS);
    bool isActive()
    {
        return !startNeeded;
    };
    void ISR()
    {
        TimerIntClear(timerBase, TIMER_TIMA_TIMEOUT);

        lastFiredUS += lastLoadUS;

        if (readIndex == writeIndex)
        {
            TimerDisable(timerBase, TIMER_A);
            TimerActionSet(TIMER_CFG_A_ACT_CLRSETTO);
            TimerActionSet(TIMER_CFG_A_ACT_NONE);
            startNeeded = true;
            lastLoadUS = 0;
        }
        else
        {
            TimerActionSet(TIMER_CFG_A_ACT_SETCLRTO * buffer[readIndex].state);
            lastLoadUS = buffer[readIndex].load;

            /*
             * The duration of the ISR adds to the load value and hence
             * influences the signal period. Dynamic profiling (measure the
             * runtime of the ISR each time and substract the last runtime)
             * sounds like the most exact way of doing it but actually causes
             * more jitter than a constant value.
             * Disadvantage: this needs careful profiling every time the ISR
             * changes. It also means that the debug-build (no optimizations)
             * will be off by a bit (since it's ISR is slower) but that doesn't
             * really matter.
             */
            TimerLoadSet(timerBase, TIMER_A, lastLoadUS * TICKS_PER_US - 180);

            readIndex = (readIndex + 1) % BUFFER_SIZE;
        }
    };
    volatile uint32_t lastFiredUS = 0;

    static void setMaxPeriodUS(uint32_t maxPeriodUS)
    {
        Output::maxPeriodUS = maxPeriodUS;
    };

private:
    struct Signal
    {
        uint32_t load;
        uint32_t state;
    };

    uint32_t bufferInsert(Signal& signal);
    uint32_t bufferLevel()
    {
        if (readIndex <= writeIndex)
        {
            return writeIndex - readIndex;
        }
        else
        {
            return BUFFER_SIZE - readIndex + writeIndex;
        }
    };
    void TimerActionSet(uint32_t act)
    {
        HWREG(timerBase + TIMER_O_TAMR) = (act >> 4) |
                                          (TIMER_CFG_PERIODIC & 0xff) |
                                           TIMER_TAMR_TAPWMIE;
    };
    static constexpr uint32_t TIMER_SYSCTL_PERIPH = 0;
    static constexpr uint32_t TIMER_BASE          = 1;
    static constexpr uint32_t PORT_SYSCTL_PERIPH  = 2;
    static constexpr uint32_t PORT_BASE           = 3;
    static constexpr uint32_t PIN                 = 4;
    static constexpr uint32_t PIN_CONFIG          = 5;
    static constexpr uint32_t TIMER_MAPPING[6][6] =
        {{SYSCTL_PERIPH_TIMER0, TIMER0_BASE, SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_PD0_T0CCP0},
         {SYSCTL_PERIPH_TIMER1, TIMER1_BASE, SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_PD2_T1CCP0},
         {SYSCTL_PERIPH_TIMER2, TIMER2_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_0, GPIO_PM0_T2CCP0},
         {SYSCTL_PERIPH_TIMER3, TIMER3_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_2, GPIO_PM2_T3CCP0},
         {SYSCTL_PERIPH_TIMER4, TIMER4_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_PM4_T4CCP0},
         {SYSCTL_PERIPH_TIMER5, TIMER5_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_6, GPIO_PM6_T5CCP0}
    };
    static constexpr uint32_t TIMER_CONFIG_POS = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_A_ACT_SETCLRTO);
    static constexpr uint32_t TICKS_PER_US = 120;
    static constexpr uint32_t MIN_TIME_US = 4;
    uint32_t minOffValue = 10;
    uint32_t maxOntimeUS = 100;
    uint32_t timerBase = 0;

    static uint32_t maxPeriodUS;

    static constexpr uint32_t BUFFER_SIZE = 1024;
    volatile Signal buffer[BUFFER_SIZE];
    volatile uint32_t readIndex = 0, writeIndex = 0, lastWriteIndex = 0;
    volatile uint32_t lastLoadUS = 0;
    volatile bool firedLastTime = false;
    volatile bool startNeeded = true;
};

#endif /* OUTPUT_H_ */
