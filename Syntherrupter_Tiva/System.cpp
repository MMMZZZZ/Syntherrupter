/*
 * System.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */

#include <System.h>


volatile uint32_t System::timeUS = 0;
volatile uint32_t System::sysTickResUS = 50;
//uint32_t System::sysTickHalfRes = sysTickResUS / 2;
constexpr uint32_t System::peripheralsCount;
constexpr uint32_t System::peripherals[peripheralsCount];



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
    //sysTickHalfRes = sysTickResUS / 2;
    SysTickPeriodSet(clockTicksUS * sysTickResUS);
}
