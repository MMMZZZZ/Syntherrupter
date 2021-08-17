/*
 * Output.cpp
 *
 *  Created on: 02.06.2021
 *      Author: Max Zuidberg
 */

#include <Output.h>


constexpr uint32_t Output::TIMER_MAPPING[6][6];
uint32_t Output::maxPeriodUS;


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
    if (TICKS_PER_US == 16)
    {
        TimerClockSourceSet(timerBase, TIMER_CLOCK_PIOSC);
    }
    TimerUpdateMode(timerBase, TIMER_A, TIMER_UP_LOAD_TIMEOUT);
    TimerLoadSet(timerBase, TIMER_A, 12000);
    TimerIntRegister(timerBase, TIMER_A, ISR);
    TimerIntEnable(timerBase, TIMER_TIMA_TIMEOUT);
    TimerActionSet(TIMER_CFG_A_ACT_NONE);

    // Configure pins for highest output current.
    GPIOPadConfigSet(TIMER_MAPPING[timerNum][PORT_BASE], TIMER_MAPPING[timerNum][PIN], GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_1);
}

void Output::setMaxOntimeUS(uint32_t maxOntimeUS)
{
    this->maxOntimeUS = maxOntimeUS;
}

void Output::addPulse(Pulse& pulse)
{
    Signal signal = {.load = 0, .state = false};
    uint32_t excess = 0;
    if (pulse.ontimeUS < MIN_TIME_US)
    {
        pulse.timeUS += pulse.ontimeUS;
        pulse.ontimeUS = 0;
    }
    if (pulse.timeUS)
    {
        signal.load  = Branchless::max(pulse.timeUS, MIN_TIME_US);
        signal.state = false;
        bufferInsert(signal);
    }
    if (pulse.ontimeUS)
    {
        signal.load  = pulse.ontimeUS;
        signal.state = true;
        excess = bufferInsert(signal);
    }
    if (excess)
    {
        // Note: excess is guaranteed to be >=MIN_TIME_US
        signal.load  = excess;
        signal.state = false;
        bufferInsert(signal);
    }
}

uint32_t Output::bufferInsert(Signal& signal)
{
    uint32_t excess = 0;
    TimerIntDisable(timerBase, TIMER_TIMA_TIMEOUT);
    /*
     * Prevent fragmentation of the buffer by merging identical states
     * together - as long as those states are ahead of time (readIndex
     * has not catched up with writeIndex).
     * This greatly reduces the load of the CPU by minimizing the
     * frequency of the ISRs (each buffer entry causes 1 ISR).
     * The fragmentation happens because Coil::updateOutput always fills
     * the buffer X ms ahead of the current position. If a 5us ontime is
     * served by this class, a 5us offtime may be added in the next run
     * of Coil::updateOutput. This would obviously lead to many small
     * entries - possibly getting smaller over time.
     */
    if (readIndex != writeIndex && buffer[lastWriteIndex].state == signal.state)
    {
        uint32_t mergedLoad = buffer[lastWriteIndex].load + signal.load;
        if (signal.state && mergedLoad > maxOntimeUS)
        {
            // Make sure the excess (which will result in an added offtime)
            // is at least as long as the required minimum.
            excess = Branchless::max(mergedLoad - maxOntimeUS, MIN_TIME_US);
            mergedLoad -= excess;
        }
        buffer[lastWriteIndex].load = mergedLoad;
    }
    else if (signal.load)
    {
        buffer[writeIndex].load  = signal.load;
        buffer[writeIndex].state = signal.state;
        lastWriteIndex = writeIndex;
        writeIndex++;
        writeIndex %= BUFFER_SIZE;
        if (writeIndex == readIndex)
        {
            readIndex++;
            readIndex %= BUFFER_SIZE;
        }
    }
    /*
     * Buffer defragmentation is nice but too long timer runtimes
     * can cause issues, too (new signals could occur within this time
     * that'll be missed).
     * To solve the issue any load values that are too high will be
     * split.
     *
     * req'd: new = high / n such that new <= thrs
     * n >= high / thrs
     * n  = high / thrs + 1
     * new = high / n
     * new = high / (high / thrs + 1)
     * new = high / ((high + thrs) / thrs)
     * new = high * thrs / (high + thrs)
     *
     * ... hmmm if the buffer get's defragmented and then refragmented
     * again... maybe I should call the whole process buffer unification?
     */
    auto* lastSignal = &(buffer[lastWriteIndex]);
    if (lastSignal->load > maxPeriodUS)
    {
        uint32_t newLoad = lastSignal->load * maxPeriodUS / (lastSignal->load + maxPeriodUS);
        do
        {
            uint32_t remaining = lastSignal->load - newLoad;
            buffer[writeIndex].load  = remaining;
            buffer[writeIndex].state = lastSignal->state;
            lastSignal->load = newLoad;
            // Not happy that I copied this... TODO
            lastWriteIndex = writeIndex;
            writeIndex++;
            writeIndex %= BUFFER_SIZE;
            if (writeIndex == readIndex)
            {
                readIndex++;
                readIndex %= BUFFER_SIZE;
            }
            lastSignal = &(buffer[lastWriteIndex]);
            // This loop can take some time. Therefore allow pending Interrupts
            // to be processed.
            TimerIntEnable(timerBase, TIMER_TIMA_TIMEOUT);
            TimerIntDisable(timerBase, TIMER_TIMA_TIMEOUT);
        } while (lastSignal->load >= newLoad + MIN_TIME_US);
    }

    TimerIntEnable(timerBase, TIMER_TIMA_TIMEOUT);

    if (startNeeded)
    {
        startNeeded = false;
        ISR();
        TimerEnable(timerBase, TIMER_A);
    }

    return excess;
}
