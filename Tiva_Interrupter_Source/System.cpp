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
     uint32_t clock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), sysClockFreq);

     if (clock != sysClockFreq)
     {
         error();
     }

     sysExactTime = 0;
     sysTime = 0;


    FPULazyStackingEnable();
    FPUEnable();

    IntMasterEnable();

    SysTickIntRegister(ISR);
    IntPrioritySet(FAULT_SYSTICK, 0b11100000);
    setSystemTimeResUS(100);
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
    sysTickHalfRes = sysTickResUS / 2;
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

uint32_t System::getExactSystemTimeUS()
{
    // Different variables needed to prevent sync issues.

    uint32_t newExactTime = sysExactTime + (SysTickPeriodGet() - SysTickValueGet()) / (sysClockFreq / 1000000);
    while ((newExactTime + sysTickHalfRes) < sysTime)
    {
        sysExactTime += sysTickResUS;
        newExactTime += sysTickResUS;
    }
    return newExactTime;

    // If ISR
}

void System::delayUS(uint32_t us)
{
    SysCtlDelay(((sysClockFreq / 1000000) * us) / 3);
}
