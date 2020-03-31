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
    void init(System* sys);
    void tone(float freq, float ontimeUS);
    void periodicISR();
    void setMaxDuty(float maxDuty);
    void setMaxDutyPerc(uint32_t maxDutyPerc);
    void setMaxOntimeUS(float maxOntimeUS);
private:
    void disable();
    void enable();
    static constexpr uint32_t OUTPUT_TIMER_SYSCTL_PERIPH       = SYSCTL_PERIPH_TIMER1;
    static constexpr uint32_t OUTPUT_TIMER_BASE                = TIMER1_BASE;
    static constexpr uint32_t OUTPUT_TIMER_CONFIG              = (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM);
    static constexpr uint32_t OUTPUT_TIMER_PORT_SYSCTL_PERIPH  = SYSCTL_PERIPH_GPIOD;
    static constexpr uint32_t OUTPUT_TIMER_PORT_BASE           = GPIO_PORTD_BASE;
    static constexpr uint32_t OUTPUT_TIMER_PIN                 = GPIO_PIN_2;
    static constexpr uint32_t OUTPUT_TIMER_PIN_CONFIG          = GPIO_PD2_T1CCP0;
    //static constexpr uint32_t OUTPUT_TIMER_PRESCALE            = 250;

    System* outputSys;
    uint32_t outputLoadValue = 0, outputMatchValue = 0, outputLoadValueNew = 0, outputMatchValueNew = 0;
    uint32_t outputMatchValueMax = 1;
    float outputDutyMaxUS = 0.01f;
    bool outputActive = false;
};

#endif /* OUTPUT_H_ */
