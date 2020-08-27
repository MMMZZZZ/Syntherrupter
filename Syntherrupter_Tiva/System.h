/*
 * System.h
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"

class System
{
public:
    System();
    virtual ~System();
    static void init(void (*ISR)(void));
    static void error();
    static void setSystemTimeResUS(uint32_t us);
    static uint32_t getClockFreq()
    {
        return clockFreq;
    };
    static uint32_t getPIOSCFreq()
    {
        return PIOSCFreq;
    };
    static void systemTimeIncrement()
    {
        timeUS += sysTickResUS;
    };
    static uint32_t getSystemTimeUS()
    {
        return timeUS;
    };
    static uint32_t getSystemTimeResUS()
    {
        return sysTickResUS;
    }
    static void delayUS(uint32_t us)
    {
        SysCtlDelay((clockTicksUS * us) / 3);
    };
    static uint32_t rand(uint32_t lower, uint32_t upper)
    {
        return ((timeUS + SysTickValueGet()) % (upper - lower) + lower);
    };
private:
    static constexpr uint32_t clockFreq = 120000000;
    static constexpr uint32_t clockTicksUS = clockFreq / 1000000;
    static constexpr uint32_t PIOSCFreq = 16000000;
    static volatile uint32_t timeUS;
    static volatile uint32_t sysTickResUS;

    // Peripherals that should be turned off in case of an error
    static constexpr uint32_t peripheralsCount = 43;
    static constexpr uint32_t peripherals[peripheralsCount] = { SYSCTL_PERIPH_EMAC0,
                                                     SYSCTL_PERIPH_EPHY0,
                                                     SYSCTL_PERIPH_EPI0,
                                                     SYSCTL_PERIPH_GPIOB,
                                                     SYSCTL_PERIPH_GPIOC,
                                                     SYSCTL_PERIPH_GPIOD,
                                                     SYSCTL_PERIPH_GPIOE,
                                                     SYSCTL_PERIPH_GPIOF,
                                                     SYSCTL_PERIPH_GPIOG,
                                                     SYSCTL_PERIPH_GPIOH,
                                                     SYSCTL_PERIPH_GPIOJ,
                                                     SYSCTL_PERIPH_HIBERNATE,
                                                     SYSCTL_PERIPH_CCM0,
                                                     SYSCTL_PERIPH_FAN0,
                                                     SYSCTL_PERIPH_FAN1,
                                                     SYSCTL_PERIPH_GPIOK,
                                                     SYSCTL_PERIPH_GPIOL,
                                                     SYSCTL_PERIPH_GPIOM,
                                                     SYSCTL_PERIPH_GPION,
                                                     SYSCTL_PERIPH_GPIOP,
                                                     SYSCTL_PERIPH_GPIOQ,
                                                     SYSCTL_PERIPH_GPIOR,
                                                     SYSCTL_PERIPH_GPIOS,
                                                     SYSCTL_PERIPH_GPIOT,
                                                     SYSCTL_PERIPH_PWM0,
                                                     SYSCTL_PERIPH_PWM1,
                                                     SYSCTL_PERIPH_QEI0,
                                                     SYSCTL_PERIPH_QEI1,
                                                     SYSCTL_PERIPH_SSI0,
                                                     SYSCTL_PERIPH_SSI1,
                                                     SYSCTL_PERIPH_SSI2,
                                                     SYSCTL_PERIPH_SSI3,
                                                     SYSCTL_PERIPH_TIMER0,
                                                     SYSCTL_PERIPH_TIMER1,
                                                     SYSCTL_PERIPH_TIMER2,
                                                     SYSCTL_PERIPH_TIMER3,
                                                     SYSCTL_PERIPH_TIMER4,
                                                     SYSCTL_PERIPH_TIMER5,
                                                     SYSCTL_PERIPH_TIMER6,
                                                     SYSCTL_PERIPH_TIMER7,
                                                     SYSCTL_PERIPH_WDOG0,
                                                     SYSCTL_PERIPH_WDOG1};
};

#endif /* SYSTEM_H_ */
