/*
 * System.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max
 */

#include <System.h>

System::System()
{
    // TODO Auto-generated constructor stub

}

System::~System()
{
    // TODO Auto-generated destructor stub
}

void System::init(uint32_t clockFreq, void (*ISR)(void))
{
    sysClockFreq = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), clockFreq);

    FPULazyStackingEnable();
    FPUEnable();

    IntMasterEnable();

    SysTickIntRegister(ISR);
    IntPrioritySet(FAULT_SYSTICK, 0b11100000);
    setSystemTimeResUS(1000);
    SysTickIntEnable();
    SysTickEnable();
}

uint32_t System::getClockFreq()
{
    return sysClockFreq;
}
uint32_t System::getPIOSCFreq()
{
    return sysPIOSCFreq;
}

void System::error()
{
    // Disable Interrupts
    // IntMasterDisable();

    // Stop all peripherals
    for (uint_fast8_t i = 0; i < sysPeripheralsCount; i++)
    {
        SysCtlPeripheralReset(sysPeripherals[i]);
        SysCtlPeripheralDisable(sysPeripherals[i]);
    }

    while (42);
}

void System::setSystemTimeResUS(uint32_t us)
{
    sysTickResUS = us;
    SysTickPeriodSet((sysClockFreq / 1000000) * sysTickResUS);
}

uint32_t System::getSystemTimeResUS()
{
    return sysTickResUS;
}

void System::systemTimeIncrement()
{
    sysTime += sysTickResUS;
}

uint32_t System::getSystemTimeUS()
{
    return sysTime;
}

float System::getExactSystemTimeUS()
{
    return float(sysTime) + float(SysTickPeriodGet() - SysTickValueGet()) / float((sysClockFreq / 1000000));
}

void System::delayUS(uint32_t us)
{
    SysCtlDelay(((sysClockFreq / 1000000) * us) / 3);
}
