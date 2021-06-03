/*
 * Output.cpp
 *
 *  Created on: 02.06.2021
 *      Author: Max Zuidberg
 */

#include <Output.h>


constexpr uint32_t Output::TIMER_MAPPING[6][6];


Output::Output()
{
    // TODO Auto-generated constructor stub

}

Output::~Output()
{
    // TODO Auto-generated destructor stub
}

void Output::init(uint32_t timerNum, void (*ISR)(void))
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
    TimerConfigure(timerBase, TIMER_CONFIG_POS);
    TimerClockSourceSet(timerBase, TIMER_CLOCK_PIOSC);
    TimerUpdateMode(timerBase, TIMER_A, TIMER_UP_LOAD_TIMEOUT | TIMER_UP_MATCH_TIMEOUT);
    TimerIntRegister(timerBase, TIMER_A, ISR);

    GPIOPinConfigure(TIMER_MAPPING[timerNum][PIN_CONFIG]);
    GPIOPinTypeTimer(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN]);

    // Configure pins for highest output current.
    GPIOPadConfigSet(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN], GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
}

void Output::setMaxOntimeUS(uint32_t maxOntimeUS)
{
    maxOnValue = maxOntimeUS * (System::getPIOSCFreq() / 1000000);
}

void Output::insert(float* times, float* ontimes, uint32_t count)
{
    for (uint32_t i = 0; i < count - 1; i++)
    {
        for (uint32_t j = 0; j <= i; j++)
        {
            if (times[j] > times[j + 1])
            {
                float temp = times[j];
                times[j] = times[j + 1];
                times[j + 1] = temp;
                temp = ontimes[j];
                ontimes[j] = ontimes[j + 1];
                ontimes[j + 1] = temp;
            }
        }
    }
    static float lastTime = 0.0f;
    float temp = times[0] - lastTime;
    lastTime = times[count - 1];
    for (uint32_t i = count - 1; i >= 1; i--)
    {
        times[i] -= times[i - 1];

    }
    times[0] = temp;

    auto& buffer = currentBuffer == 0 ? buffer1 : buffer0;
    uint32_t index = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        buffer[index].load = times[i] * 16;
        buffer[index].match = ontimes[i] * 16;
    }
}
