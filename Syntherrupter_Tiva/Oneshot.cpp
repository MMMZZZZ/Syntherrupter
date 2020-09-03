/*
 * Oneshot.cpp
 *
 *  Created on: 08.05.2020
 *      Author: Max Zuidberg
 */

#include <Oneshot.h>


constexpr uint32_t Oneshot::TIMER_MAPPING[6][6];


Oneshot::Oneshot()
{
    // TODO Auto-generated constructor stub

}

Oneshot::~Oneshot()
{
    // TODO Auto-generated destructor stub
}

void Oneshot::init(uint32_t timerNum)
{
    // Timer base stored separately for better readability (used very often).
    timerBase = TIMER_MAPPING[timerNum][TIMER_BASE];

    SysCtlPeripheralEnable(TIMER_MAPPING[timerNum][TIMER_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(TIMER_MAPPING[timerNum][PORT_SYSCTL_PERIPH]);
    SysCtlDelay(2);

    // In case timer was previously configured differently
    SysCtlPeripheralReset(TIMER_MAPPING[timerNum][TIMER_SYSCTL_PERIPH]);

    // Make sure the GPIO Pin is inactive while (re)configuring the timer
    GPIOPinTypeGPIOOutput(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN]);
    GPIOPinWrite(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN], 0);

    // Timer A generates the ontime, timer B assures enough offtime until the next pulse
    TimerConfigure(timerBase, TIMER_CONFIG);
    TimerClockSourceSet(timerBase, TIMER_CLOCK_PIOSC);

    shot(10);

    // wait until timer has stopped (a.k.a. disabled itself)
    while (HWREG(timerBase + TIMER_O_CTL) & TIMER_CTL_TAEN);

    GPIOPinConfigure(TIMER_MAPPING[timerNum][PIN_CONFIG]);
    GPIOPinTypeTimer(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN]);

    // Configure pins for highest output current.
    GPIOPadConfigSet(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN], GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
}

void Oneshot::setMaxOntimeUS(uint32_t maxOntimeUS)
{
    maxOnValue = maxOntimeUS * (System::getPIOSCFreq() / 1000000);
}
