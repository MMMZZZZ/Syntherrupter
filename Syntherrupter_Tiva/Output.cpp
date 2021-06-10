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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
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

void Output::insert(Pulse* pulses, uint32_t count, int32_t bufferTime)
{
    // Replace timeUS (relative to start time of this buffer) by times relative
    // to the last action (on-/offtime). Overlapping shouldn't occur but it is
    // catched by setting the relative time to 0;
    for (uint32_t i = count - 1; i >= 1; i--)
    {
        uint32_t temp = pulses[i - 1].timeUS + pulses[i - 1].ontimeUS;
        if (temp >= pulses[i].timeUS)
        {
            pulses[i].timeUS = 0;
        }
        else
        {
            pulses[i].timeUS -= temp;
        }
    }

    Signal tempBuffer[BUFFER_SIZE];
    uint32_t index = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        if (index > BUFFER_SIZE - 1)
        {
            break;
        }
        if (pulses[i].timeUS)
        {
            tempBuffer[index].load  = pulses[i].timeUS * 120;
            tempBuffer[index].state = false;
            index++;
        }
        if (pulses[i].ontimeUS)
        {
            tempBuffer[index].load  = pulses[i].ontimeUS * 120;
            tempBuffer[index].state = true;
            index++;
        }
    }

    TimerIntDisable(timerBase, TIMER_TIMA_TIMEOUT);
    auto& buffer = size0 == 0 ? buffer0 : buffer1;
    auto& size   = size0 == 0 ? size0   : size1;
    auto& otherSize = size0 == 0 ? size1   : size0;
    for (uint32_t i = 0; i < index; i++)
    {
        buffer[i].load  = tempBuffer[i].load;
        buffer[i].state = tempBuffer[i].state;
    }
    size = index;
    if (otherSize)
    {
        wannaMore = false;
        if (startNeeded)
        {
            ISR();
            TimerEnable(timerBase, TIMER_A);
            startNeeded = false;
        }
    }
    else
    {
        startNeeded = true;
    }
    TimerIntEnable(timerBase, TIMER_TIMA_TIMEOUT);
}
