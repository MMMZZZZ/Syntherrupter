/*
 * Oneshot.cpp
 *
 *  Created on: 08.05.2020
 *      Author: Max Zuidberg
 */

#include <Oneshot.h>


Oneshot::Oneshot()
{
    // TODO Auto-generated constructor stub

}

Oneshot::~Oneshot()
{
    // TODO Auto-generated destructor stub
}

void Oneshot::init(System *sys, uint32_t timerNum)
{
    oneshotSys = sys;
    oneshotTimerNum = timerNum;

    // Timer base stored separately for better readability (used very often).
    oneshotTimerBase = ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_TIMER_BASE];

    SysCtlPeripheralEnable(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_TIMER_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PORT_SYSCTL_PERIPH]);
    SysCtlDelay(2);

    // In case timer was previously configured differently
    SysCtlPeripheralReset(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_TIMER_SYSCTL_PERIPH]);

    // Make sure the GPIO Pin is inactive while (re)configuring the timer
    GPIOPinTypeGPIOOutput(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PORT_BASE], ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PIN]);
    GPIOPinWrite(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PORT_BASE], ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PIN], 0);

    // Timer A generates the ontime, timer B assures enough offtime until the next pulse
    TimerConfigure(oneshotTimerBase, ONESHOT_TIMER_CONFIG);
    TimerClockSourceSet(oneshotTimerBase, TIMER_CLOCK_PIOSC);

    shot(10);

    // wait until timer has stopped (a.k.a. disabled itself)
    while (HWREG(oneshotTimerBase + TIMER_O_CTL) & TIMER_CTL_TAEN);

    GPIOPinConfigure(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PIN_CONFIG]);
    GPIOPinTypeTimer(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PORT_BASE], ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PIN]);

    // Configure pins for highest output current.
    GPIOPadConfigSet(ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PORT_BASE], ONESHOT_MAPPING[oneshotTimerNum][ONESHOT_PIN], GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
}

void Oneshot::rearmISR()
{
    TimerIntClear(oneshotTimerBase, TIMER_CAPA_EVENT);
    TimerDisable(oneshotTimerBase, TIMER_A);
    //oneshotReady = true;
}

void Oneshot::setMaxOntimeUS(uint32_t maxOntimeUS)
{
    oneshotMaxOnValue = maxOntimeUS * (oneshotSys->getPIOSCFreq() / 1000000);
}

void Oneshot::setMinOfftimeUS(uint32_t minOfftimeUS)
{
    if (minOfftimeUS)
    {
        oneshotMinOffValue = minOfftimeUS * (oneshotSys->getPIOSCFreq() / 1000000);
    }
    else
    {
        oneshotMinOffValue = 1;
    }
}

void Oneshot::shot(uint32_t ontimeUS)
{
    uint32_t matchValue = ontimeUS * (oneshotSys->getPIOSCFreq() / 1000000);
    if (matchValue)
    {
        if (matchValue > oneshotMaxOnValue)
        {
            matchValue = oneshotMaxOnValue;
        }

        // Copied from the TivaWare timer.c, reduced to the minimum, skipping argument checks and platform checks. Big speed increase.
        // Disable the Output-Timer
        HWREG(oneshotTimerBase + TIMER_O_CTL) &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);

        // Reset the Timer A Mode Register (not sure why this is needed)
        HWREG(oneshotTimerBase + TIMER_O_TAMR) = 0;//(((0 & 0x000f0000) >> 4) | (0 & 0xff) | TIMER_TAMR_TAPWMIE);

        // Set the configuration of the A timer and set the TxPWMIE bit.
        HWREG(oneshotTimerBase + TIMER_O_TAMR) = (((ONESHOT_TIMER_CONFIG & 0x000f0000) >> 4) | (ONESHOT_TIMER_CONFIG & 0xff) | TIMER_TAMR_TAPWMIE);

        // Set Load in Output-Timer Timer A Interval Load Register
        HWREG(oneshotTimerBase + TIMER_O_TAILR) = matchValue - 1;

        // Enable the Timer
        HWREG(oneshotTimerBase + TIMER_O_CTL) |= TIMER_CTL_TAEN;
    }
}
