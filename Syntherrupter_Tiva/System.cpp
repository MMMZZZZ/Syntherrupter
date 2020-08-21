/*
 * System.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
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

void System::init(void (*ISR)(void))
{
     uint32_t clock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), clockFreq);

     if (clock != clockFreq)
     {
         error();
     }

     timeUS = 0;


    FPULazyStackingEnable();
    FPUFlushToZeroModeSet(FPU_FLUSH_TO_ZERO_EN);
    FPUEnable();

    IntMasterEnable();

    SysTickIntRegister(ISR);
    IntPrioritySet(FAULT_SYSTICK, 0b00000000);
    setSystemTimeResUS(100);
    SysTickIntEnable();
    SysTickEnable();
}

uint32_t System::getClockFreq()
{
    return clockFreq;
}
uint32_t System::getPIOSCFreq()
{
    return PIOSCFreq;
}

void System::error()
{
    // Disable Interrupts
    // IntMasterDisable();

    // Stop all peripherals
    for (uint_fast8_t i = 0; i < peripheralsCount; i++)
    {
        SysCtlPeripheralReset(peripherals[i]);
        SysCtlPeripheralDisable(peripherals[i]);
    }

    while (42);
}

void System::setSystemTimeResUS(uint32_t us)
{
    sysTickResUS = us;
    sysTickHalfRes = sysTickResUS / 2;
    SysTickPeriodSet(clockTicksUS * sysTickResUS);
}

uint32_t System::getSystemTimeResUS()
{
    return sysTickResUS;
}

void System::systemTimeIncrement()
{
    timeUS += sysTickResUS;
}

uint32_t System::getSystemTimeUS()
{
    return timeUS;
}

void System::delayUS(uint32_t us)
{
    SysCtlDelay((clockTicksUS * us) / 3);
}

uint32_t System::rand(uint32_t lower, uint32_t upper)
{
    return ((timeUS + SysTickValueGet()) % (upper - lower) + lower);
}
