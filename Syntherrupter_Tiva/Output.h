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
#include "System.h"
#include "Buffer.h"
#include "Pulse.h"


class Output
{
public:
    Output();
    virtual ~Output();
    void init(uint32_t timer, void (*ISR)(void));
    void insert(Pulse* pulses, uint32_t count, int32_t bufferTime);
    void setMaxOntimeUS(uint32_t maxOntimeUS);
    void setMinOfftimeUS(uint32_t minOfftimeUS);
    bool requiresData()
    {
        return wannaMore;
    };
    bool isRunning()
    {
        return (size0 || size1);
    };
    void ISR()
    {
        //GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_1, 0xff);
        TimerIntClear(timerBase, TIMER_TIMA_TIMEOUT);
        if (!buffer)
        {
            if (size0)
            {
                buffer = 1;
            }
            else if (size1)
            {
                buffer = 2;
            }
            else
            {
                TimerActionSet(TIMER_CFG_A_ACT_CLRTOGTO);
                TimerDisable(timerBase, TIMER_A);
            }
        }
        if (buffer == 1)
        {
            if (buffer0[index0].state)
            {
                TimerActionSet(TIMER_CFG_A_ACT_SETCLRTO);
                if (firedLastTime && timerBase == TIMER2_BASE)
                {
                    firedLastTime = 2;
                }
                firedLastTime = true;
            }
            else
            {
                TimerActionSet(TIMER_CFG_A_ACT_NONE);
                if (!firedLastTime && timerBase == TIMER2_BASE)
                {
                    firedLastTime = 0;
                }
                firedLastTime = false;
            }
            TimerLoadSet(timerBase, TIMER_A, buffer0[index0].load);

            if (++index0 >= size0)
            {
                buffer = 0;
                index0 = 0;
                size0  = 0;
                wannaMore = 1;
            }
        }
        else if (buffer == 2)
        {
            if (buffer1[index1].state)
            {
                TimerActionSet(TIMER_CFG_A_ACT_SETCLRTO);
            }
            else
            {
                TimerActionSet(TIMER_CFG_A_ACT_NONE);
            }
            TimerLoadSet(timerBase, TIMER_A, buffer1[index1].load);
            if (++index1 >= size1)
            {
                buffer = 0;
                index1 = 0;
                size1  = 0;
                wannaMore = 1;
            }
        }
        //GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_1, 0x00);
    };

private:
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
    static constexpr uint32_t TIMER_CONFIG_NEG = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_A_ACT_CLRSETTO);
    uint32_t minOffValue = 160;
    uint32_t maxOnValue = 1600;
    uint32_t timerBase = 0;

    bool startNeeded = false;
    static constexpr uint32_t BUFFER_SIZE = 256;
    struct Signal
    {
        uint32_t load;
        uint32_t state;
    };
    volatile Signal buffer0[BUFFER_SIZE], buffer1[BUFFER_SIZE];
    volatile uint32_t index0 = 0, index1 = 0, size0 = 0, size1 = 0;
    volatile uint32_t wannaMore = 1;
    volatile uint32_t buffer = 0;
    volatile bool firedLastTime = false;
};

#endif /* OUTPUT_H_ */
