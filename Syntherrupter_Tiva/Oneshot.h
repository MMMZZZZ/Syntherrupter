/*
 * Oneshot.h
 *
 *  Created on: 08.05.2020
 *      Author: Max Zuidberg
 */

#ifndef ONESHOT_H_
#define ONESHOT_H_


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

// Make sure inversion doesn't slip into public releases!
#ifndef TEABUG
#ifdef INVERT
static_assert(false, "INVERT not allowed for release builds.");
#endif /* INVERT */
#endif /* TEABUG */

class Oneshot
{
public:
    Oneshot();
    virtual ~Oneshot();
    void init(uint32_t timer);
    void setMaxOntimeUS(uint32_t maxOntimeUS);
    void setMinOfftimeUS(uint32_t minOfftimeUS);
    void shot(uint32_t ontimeUS)
    {
        uint32_t matchValue = ontimeUS * (System::getPIOSCFreq() / 1000000);
        if (matchValue)
        {
            if (matchValue > maxOnValue)
            {
                matchValue = maxOnValue;
            }

            // Copied from the TivaWare timer.c, reduced to the minimum, skipping argument checks and platform checks. Big speed increase.
            // Disable the Output-Timer
            HWREG(timerBase + TIMER_O_CTL) &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);

            // Reset the Timer A Mode Register (not sure why this is needed)
            HWREG(timerBase + TIMER_O_TAMR) = 0;//(((0 & 0x000f0000) >> 4) | (0 & 0xff) | TIMER_TAMR_TAPWMIE);

            // Set the configuration of the A timer and set the TxPWMIE bit.
            HWREG(timerBase + TIMER_O_TAMR) = (((TIMER_CONFIG & 0x000f0000) >> 4) | (TIMER_CONFIG & 0xff) | TIMER_TAMR_TAPWMIE);

            // Set Load in Output-Timer Timer A Interval Load Register
            HWREG(timerBase + TIMER_O_TAILR) = matchValue - 1;

            // Enable the Timer
            HWREG(timerBase + TIMER_O_CTL) |= TIMER_CTL_TAEN;
        }
};
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
#ifdef INVERT
    static constexpr uint32_t TIMER_CONFIG = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_A_ACT_CLRSETTO); //TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM;
    static constexpr uint32_t DEFAULT_STATE = 0xff;
#else
    static constexpr uint32_t TIMER_CONFIG = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_A_ACT_SETCLRTO); //TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM;
    static constexpr uint32_t DEFAULT_STATE = 0x00;
#endif
    uint32_t minOffValue = 160;
    uint32_t maxOnValue = 1600;
    uint32_t timerBase = 0;
};

#endif /* H_ */
