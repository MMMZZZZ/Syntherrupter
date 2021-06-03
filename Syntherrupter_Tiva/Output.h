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
#include "System.h"
#include "Buffer.h"


class Output
{
public:
    Output();
    virtual ~Output();
    void init(uint32_t timer, void (*ISR)(void));
    void insert(float* periods, float* ontimes, uint32_t count);
    void setMaxOntimeUS(uint32_t maxOntimeUS);
    void setMinOfftimeUS(uint32_t minOfftimeUS);
    void ISR()
    {
        TimerIntClear(timerBase, TIMER_TIMA_TIMEOUT);
        if (currentBuffer == 0)
        {
            TimerLoadSet(timerBase, TIMER_A, buffer0[index0].load);
            TimerMatchSet(timerBase, TIMER_A, buffer0[index0].match);
            if (++index0 >= BUFFER_SIZE)
            {
                index0 = 0;
                currentBuffer = 1;
            }
        }
        else
        {
            TimerLoadSet(timerBase, TIMER_A, buffer0[index1].load);
            TimerMatchSet(timerBase, TIMER_A, buffer0[index1].match);
            if (++index1 >= BUFFER_SIZE)
            {
                index1 = 0;
                currentBuffer = 0;
            }
        }
    };
    static constexpr uint32_t BUFFER_SIZE = 64;
    struct Signal
    {
        uint32_t load;
        uint32_t match;
    };
    volatile Signal buffer0[BUFFER_SIZE], buffer1[BUFFER_SIZE];
    volatile uint32_t currentBuffer;

private:
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
    static constexpr uint32_t TIMER_CONFIG_NEG = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_A_ACT_CLRSETTO);
    uint32_t minOffValue = 160;
    uint32_t maxOnValue = 1600;
    uint32_t timerBase = 0;

    volatile uint32_t index0 = 0, index1 = 0;
};

#endif /* OUTPUT_H_ */
