/*
 * Output.cpp
 *
 *  Created on: 28.03.2020
 *      Author: Max Zuidberg
 */

#include <Output.h>

Output::Output()
{
    // TODO Auto-generated constructor stub

}

Output::~Output()
{
    // TODO Auto-generated destructor stub
}

void Output::init(System* sys, uint32_t timerNum)
{
    outputSys = sys;
    outputTimerNum = timerNum;

    // Timer base stored separately for better readability (used very often).
    outputTimerBase = OUTPUT_MAPPING[outputTimerNum][OUTPUT_TIMER_BASE];

    // Allow reinitialization of timer and GPIO hardware to switch between oneshot and periodic operation.
    // There might be better ways to do, this was however the easiest with the existing code.
    SysCtlPeripheralEnable(OUTPUT_MAPPING[outputTimerNum][OUTPUT_TIMER_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PORT_SYSCTL_PERIPH]);
    SysCtlDelay(2);

    // In case timer was previously configured differently
    SysCtlPeripheralReset(OUTPUT_MAPPING[outputTimerNum][OUTPUT_TIMER_SYSCTL_PERIPH]);

    // Make sure the GPIO Pin is inactive while (re)configuring the timer
    GPIOPinTypeGPIOOutput(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PORT_BASE], OUTPUT_MAPPING[outputTimerNum][OUTPUT_PIN]);
    GPIOPinWrite(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PORT_BASE], OUTPUT_MAPPING[outputTimerNum][OUTPUT_PIN], 0);

    TimerConfigure(outputTimerBase, OUTPUT_TIMER_CONFIG);
    TimerClockSourceSet(outputTimerBase, TIMER_CLOCK_PIOSC);

    // Invert output which means enable on match, clear on reload.
    TimerControlLevel(outputTimerBase, TIMER_A, true);
    TimerUpdateMode(outputTimerBase, TIMER_A, TIMER_UP_LOAD_TIMEOUT);

    // Make sure output is low before enabling the GPIO Output
    TimerPrescaleSet(outputTimerBase, TIMER_A, 0);
    TimerLoadSet(outputTimerBase, TIMER_A, 0xffff);
    TimerPrescaleMatchSet(outputTimerBase, TIMER_A, 0xff);
    TimerMatchSet(outputTimerBase, TIMER_A, 0xff);
    TimerEnable(outputTimerBase, TIMER_A);
    while (GPIOPinRead(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PORT_BASE], OUTPUT_MAPPING[outputTimerNum][OUTPUT_PIN]));
    TimerDisable(outputTimerBase, TIMER_A);
    outputLoadValue = 0;
    outputMatchValue = 0;
    outputActive = false;

    GPIOPinConfigure(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PIN_CONFIG]);
    GPIOPinTypeTimer(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PORT_BASE], OUTPUT_MAPPING[outputTimerNum][OUTPUT_PIN]);

    outputMinFreq = float(outputSys->getPIOSCFreq()) / float((1 << 24) - 1);
}

void Output::setMaxDuty(float maxDuty)
{
    outputDutyMaxUS = maxDuty * 1000000.0f;
}

void Output::setMaxDutyPerm(uint32_t maxDutyPerm)
{
    outputDutyMaxUS = float(maxDutyPerm) * 1000000.0f / 1000.0f;
}

void Output::setMaxOntimeUS(float maxOntimeUS)
{
    outputMatchValueMax = float(outputSys->getPIOSCFreq() / 1000000) * maxOntimeUS;;
}

void Output::tone(float freq, float ontimeUS)
{
    if (freq < outputMinFreq || ontimeUS < 1.0f)
    {
        if (outputActive)
        {
            disable();
            outputActive = false;
        }
    }
    else
    {
        if (freq * ontimeUS > outputDutyMaxUS)
        {
            ontimeUS = outputDutyMaxUS / freq;
        }
        outputLoadValueNew = float(outputSys->getPIOSCFreq()) / freq - 1;
        outputMatchValueNew  = float(outputSys->getPIOSCFreq() / 1000000) * ontimeUS;
        if (outputMatchValueNew > outputMatchValueMax)
        {
            outputMatchValueNew = outputMatchValueMax;
        }
        if (outputLoadValueNew > outputMatchValueNew && outputMatchValueNew > 0)
        {
            if (outputLoadValue != outputLoadValueNew)
            {
                outputLoadValue = outputLoadValueNew;
                TimerPrescaleSet(outputTimerBase, TIMER_A, ((outputLoadValueNew & 0xff0000) >> 16));
                TimerLoadSet(outputTimerBase, TIMER_A, (outputLoadValueNew & 0xffff));
            }
            if (outputMatchValue != outputMatchValueNew)
            {
                outputMatchValue = outputMatchValueNew ;
                TimerPrescaleMatchSet(outputTimerBase, TIMER_A, ((outputMatchValueNew & 0xff0000) >> 16));
                TimerMatchSet(outputTimerBase, TIMER_A, (outputMatchValueNew & 0xffff));
            }
            if (!outputActive)
            {
                outputActive = true;
                TimerEnable(outputTimerBase, TIMER_A);
            }
        }
        else
        {
            disable();
            outputActive = false;
        }
    }
}

void Output::disable()
{
    TimerPrescaleMatchSet(outputTimerBase, TIMER_A, 0);
    TimerMatchSet(outputTimerBase, TIMER_A, 100);
    while (GPIOPinRead(OUTPUT_MAPPING[outputTimerNum][OUTPUT_PORT_BASE], OUTPUT_MAPPING[outputTimerNum][OUTPUT_PIN]));
    TimerDisable(outputTimerBase, TIMER_A);
    outputLoadValue = 0;
    outputMatchValue = 0;
}
