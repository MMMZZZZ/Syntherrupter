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
uint32_t System::fwVersionNum = 0;



System::System()
{
    // TODO Auto-generated constructor stub

}

System::~System()
{
    // TODO Auto-generated destructor stub
}

void System::init()
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

    SysTickIntRegister(System::systemTimeIncrement);
    IntPrioritySet(FAULT_SYSTICK, 0b00000000);
    SysTickPeriodSet(CLOCK_TICKS_US * SYS_TICK_RES_US);
    SysTickIntEnable();
    SysTickEnable();

    fwVersionNum = fwVersionToInt();
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

uint32_t System::fwVersionToInt()
{
    /*
     * Parse firmware version string into a ui32 such that "v4.2.0-beta.26"
     * becomes
     * (4 << 24) + (2 << 16) + (0 << 8) + (26 or 255)
     *
     * The 26 or 255 is necessary to make sure that the ui32 for
     * "v4.2.0" is higher than the ui32 for "v4.2.0-beta.x" - a.k.a. newer
     * firmwares always have higher version numbers. If there's no beta version
     * (a.k.a. beta.0 which isn't a valid beta version number), the lowest byte
     * becomes 255 (and thus the stable release a higher value than any
     * preceding beta releases).
     */
    uint32_t intStart = 0, intEnd = 0, intCounter = 0;
    uint32_t fwVersionNum = 0;
    for (uint32_t i = 0; i < sizeof(TIVA_FW_VERSION); i++)
    {
        volatile uint8_t a = TIVA_FW_VERSION[i];
        if (TIVA_FW_VERSION[i] >= '0' && TIVA_FW_VERSION[i] <= '9')
        {
            intEnd++;
        }
        else
        {
            if (intEnd > intStart)
            {
                uint8_t temp = 0;
                for (uint32_t j = intStart; j < intEnd; j++)
                {
                    volatile uint8_t b = TIVA_FW_VERSION[j];
                    temp *= 10;
                    temp += TIVA_FW_VERSION[j] - '0';
                }
                fwVersionNum <<= 8;
                fwVersionNum += temp;

                intCounter++;
                if (intCounter > 4)
                {
                    // Shouldn't happen but who knows.
                    break;
                }
            }
            intStart = i + 1;
            intEnd = intStart;
        }
    }

    // Make sure shorter version numbers (f.ex. non-beta versions or "v1.1")
    // get shifted to the same place.
    for (uint32_t i = intCounter; i < 4; i++)
    {
        fwVersionNum <<= 8;
    }

    // Adjust beta version part according to the notes above
    uint32_t beta = fwVersionNum & 0xff;
    if (!beta)
    {
        fwVersionNum += 0xff;
    }

    return fwVersionNum;
}
