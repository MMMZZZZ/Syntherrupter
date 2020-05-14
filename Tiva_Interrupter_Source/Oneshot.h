/*
 * Oneshot.h
 *
 *  Created on: 08.05.2020
 *      Author: Max
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
#include "System.h"


class Oneshot
{
public:
    Oneshot();
    virtual ~Oneshot();
    void init(System *sys, uint32_t timer, void (*ISR)(void));
    void setMaxOntimeUS(uint32_t maxOntimeUS);
    void setMinOfftimeUS(uint32_t minOfftimeUS);
    void shot(uint32_t ontimeUS);
    void rearmISR();
    bool ready();
private:
    static constexpr uint32_t ONESHOT_TIMER_SYSCTL_PERIPH = 0;
    static constexpr uint32_t ONESHOT_TIMER_BASE          = 1;
    static constexpr uint32_t ONESHOT_PORT_SYSCTL_PERIPH  = 2;
    static constexpr uint32_t ONESHOT_PORT_BASE           = 3;
    static constexpr uint32_t ONESHOT_PIN                 = 4;
    static constexpr uint32_t ONESHOT_PIN_CONFIG          = 5;
    const uint32_t ONESHOT_MAPPING[6][6] =
        {{SYSCTL_PERIPH_TIMER0, TIMER0_BASE, SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_PD0_T0CCP0},
         {SYSCTL_PERIPH_TIMER1, TIMER1_BASE, SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_PD2_T1CCP0},
         {SYSCTL_PERIPH_TIMER2, TIMER2_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_0, GPIO_PM0_T2CCP0},
         {SYSCTL_PERIPH_TIMER3, TIMER3_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_2, GPIO_PM2_T3CCP0},
         {SYSCTL_PERIPH_TIMER4, TIMER4_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_PM4_T4CCP0},
         {SYSCTL_PERIPH_TIMER5, TIMER5_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_6, GPIO_PM6_T5CCP0}
    };
    static constexpr uint32_t ONESHOT_TIMER_CONFIG = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_A_ACT_SETCLRTO); //TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM;

    System* oneshotSys;
    uint32_t oneshotMinOffValue = 160;
    uint32_t oneshotMaxOnValue = 1600;
    uint32_t oneshotTimerNum = 0, oneshotTimerBase = 0;
};

#endif /* ONESHOT_H_ */
