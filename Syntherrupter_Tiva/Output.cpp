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
    GPIOPinConfigure(TIMER_MAPPING[timerNum][PIN_CONFIG]);
    GPIOPinTypeTimer(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN]);

    // Timer A generates the ontime, timer B assures enough offtime until the next pulse
    TimerConfigure(timerBase, TIMER_CFG_PERIODIC);
    TimerControlStall(timerBase, TIMER_A, true);
    //TimerClockSourceSet(timerBase, TIMER_CLOCK_PIOSC);
    TimerUpdateMode(timerBase, TIMER_A, TIMER_UP_LOAD_TIMEOUT);
    TimerLoadSet(timerBase, TIMER_A, 12000);
    TimerIntRegister(timerBase, TIMER_A, ISR);
    TimerIntEnable(timerBase, TIMER_TIMA_TIMEOUT);
    TimerActionSet(TIMER_CFG_A_ACT_NONE);

    // Configure pins for highest output current.
    //GPIOPadConfigSet(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN], GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_1);
}

void Output::setMaxOntimeUS(uint32_t maxOntimeUS)
{
    maxOnValue = maxOntimeUS * (System::getPIOSCFreq() / 1000000);
}

void Output::insert(float* times, float* ontimes, uint32_t count, float bufferTime)
{
    for (int32_t i = count - 1; i >= 0; i--)
    {
        for (int32_t j = 0; j < i; j++)
        {
            if (times[j] > times[i])
            {
                float temp = times[j];
                times[j] = times[i];
                times[i] = temp;
                temp = ontimes[j];
                ontimes[j] = ontimes[i];
                ontimes[i] = temp;
            }
        }
    }

    static int32_t delayTilBufferEnd = 0;
    int32_t newDelayTilBufferEnd = bufferTime - times[count - 1] - ontimes[count - 1];

    for (uint32_t i = count - 1; i >= 1; i--)
    {
        uint32_t temp = times[i - 1] + ontimes[i - 1];
        if (temp >= times[i])
        {
            times[i] = 0;
        }
        else
        {
            times[i] -= temp;
        }
    }

    if (delayTilBufferEnd < 0 && -delayTilBufferEnd > times[0])
    {
        times[0] = 0;
    }
    else
    {
        times[0] += delayTilBufferEnd;
    }
    delayTilBufferEnd = newDelayTilBufferEnd;

    Signal tempBuffer[BUFFER_SIZE];
    uint32_t index = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        if (index > BUFFER_SIZE - 1)
        {
            break;
        }
        if (times[i])
        {
            tempBuffer[index].load  = times[i] * 120;
            tempBuffer[index].state = false;
            index++;
        }
        if (ontimes[i])
        {
            tempBuffer[index].load  = ontimes[i] * 120;
            tempBuffer[index].state = true;
            index++;
        }
    }
    if (tempBuffer[index - 1].state)
    {
        if (index >= BUFFER_SIZE)
        {
            index = BUFFER_SIZE - 1;
            delayTilBufferEnd += tempBuffer[BUFFER_SIZE - 1].load;
        }
        delayTilBufferEnd -= 2;
        tempBuffer[index].load = 240;
        tempBuffer[index].state = false;
        index++;
    }

    if (size0 || size1)
    {
        // Timer is running => sync.
        fired = false;
        while (!fired);
    }
    TimerIntDisable(timerBase, TIMER_TIMA_TIMEOUT);
    auto& buffer = size0 == 0 ? buffer0 : buffer1;
    auto& size   = size0 == 0 ? size0   : size1;
    auto& otherSize = size0 == 0 ? size1   : size0;
    size = index;
    for (uint32_t i = 0; i < size; i++)
    {
        buffer[i].load  = tempBuffer[i].load;
        buffer[i].state = tempBuffer[i].state;
    }
    if (otherSize)
    {
        wannaMore = false;
    }
    else
    {
        ISR();
        TimerEnable(timerBase, TIMER_A);
    }
    TimerIntEnable(timerBase, TIMER_TIMA_TIMEOUT);
}
