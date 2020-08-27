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

void Oneshot::shot(uint32_t ontimeUS)
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
}
