/*
 * System.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */

#include <System.h>


volatile  uint32_t System::timeUS = 0;
constexpr uint32_t System::PERIPH_COUNT;
constexpr uint32_t System::ALL_PERIPHS[PERIPH_COUNT];



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
     uint32_t clock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

     if (clock != CLOCK_FREQ)
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
    SysTickPeriodSet(CLOCK_TICKS_US * SYS_TICK_RES_US);
    SysTickIntEnable();
    SysTickEnable();
}

void System::error()
{
    // Disable Interrupts
    // IntMasterDisable();

    // Stop all peripherals
    for (uint_fast8_t i = 0; i < PERIPH_COUNT; i++)
    {
        SysCtlPeripheralReset(ALL_PERIPHS[i]);
        SysCtlPeripheralDisable(ALL_PERIPHS[i]);
    }

    while (42);
}
