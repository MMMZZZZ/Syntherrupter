/*
 * Output.h
 *
 *  Created on: 28.03.2020
 *      Author: Max
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"              // Macros defining the memory map of the Tiva C Series device. This includes defines such as peripheral base address locations such as GPIO_PORTF_BASE.
#include "inc/hw_types.h"               // Defines common types and macros.
#include "inc/hw_gpio.h"                // Defines and Macros for GPIO hardware.
#include "inc/hw_timer.h"                // Defines and macros used when accessing the UART.
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"          // Mapping of peripherals to pins for all parts.
#include "driverlib/sysctl.h"           // Defines and macros for System Control API of DriverLib. This includes API functions such as SysCtlClockSet.
#include "driverlib/gpio.h"             // Defines and macros for GPIO API of DriverLib. This includes API functions such as GPIOPinWrite.
#include "driverlib/interrupt.h"        // Defines and macros for NVIC Controller (Interrupt) API of driverLib. This includes API functions such as IntEnable and IntPrioritySet.
#include "driverlib/timer.h"             // Defines and macros for UART API of driverLib.
#include "System.h"

class Output
{
public:
    Output();
    virtual ~Output();
    void init(System* sys, uint32_t timerNum);
    void tone(float freq, float ontimeUS);
    void setMaxDuty(float maxDuty);
    void setMaxDutyPerc(uint32_t maxDutyPerc);
    void setMaxOntimeUS(float maxOntimeUS);
private:
    void disable();
    static constexpr uint32_t OUTPUT_TIMER_SYSCTL_PERIPH = 0;
    static constexpr uint32_t OUTPUT_TIMER_BASE          = 1;
    static constexpr uint32_t OUTPUT_PORT_SYSCTL_PERIPH  = 2;
    static constexpr uint32_t OUTPUT_PORT_BASE           = 3;
    static constexpr uint32_t OUTPUT_PIN                 = 4;
    static constexpr uint32_t OUTPUT_PIN_CONFIG          = 5;
    const uint32_t OUTPUT_MAPPING[6][6] =
        {{SYSCTL_PERIPH_TIMER0, TIMER0_BASE, SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_PD0_T0CCP0},
         {SYSCTL_PERIPH_TIMER1, TIMER1_BASE, SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_PD2_T1CCP0},
         {SYSCTL_PERIPH_TIMER2, TIMER2_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_0, GPIO_PM0_T2CCP0},
         {SYSCTL_PERIPH_TIMER3, TIMER3_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_2, GPIO_PM2_T3CCP0},
         {SYSCTL_PERIPH_TIMER4, TIMER4_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_PM4_T4CCP0},
         {SYSCTL_PERIPH_TIMER5, TIMER5_BASE, SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_6, GPIO_PM6_T5CCP0}
    };
    static constexpr uint32_t OUTPUT_TIMER_CONFIG = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM);

    System* outputSys;
    uint32_t outputTimerNum = 0, outputTimerBase = 0;
    uint32_t outputLoadValue = 0, outputMatchValue = 0, outputLoadValueNew = 0, outputMatchValueNew = 0;
    uint32_t outputMatchValueMax = 1;
    float outputDutyMaxUS = 0.01f;
    float outputMinFreq = 100.0f;
    bool outputActive = true;
};

#endif /* OUTPUT_H_ */
