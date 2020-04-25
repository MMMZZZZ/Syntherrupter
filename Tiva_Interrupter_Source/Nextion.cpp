/*
 * Nextion.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max
 */

#include <Nextion.h>

Nextion::Nextion()
{
    // TODO Auto-generated constructor stub

}

Nextion::~Nextion()
{
    // TODO Auto-generated destructor stub
}

void Nextion::init(System* sys, uint32_t uartNumber, uint32_t baudRate, uint32_t timeoutUS)
{
    nxtSys = sys;
    nxtBaudRate = baudRate;
    nxtTimeoutUS = timeoutUS;

    // Nextion UART stdio setup
    nxtUARTNum = uartNumber;

    SysCtlPeripheralEnable(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_PORT_SYSCTL_PERIPH]);
    while (!SysCtlPeripheralReady(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_PORT_SYSCTL_PERIPH]));
    GPIOPinConfigure(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_RX_PIN_CFG]);
    GPIOPinConfigure(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_TX_PIN_CFG]);
    GPIOPinTypeUART(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_PORT_BASE],
                    NXT_UART_MAPPING[nxtUARTNum][NXT_UART_TX_PIN] |
                    NXT_UART_MAPPING[nxtUARTNum][NXT_UART_RX_PIN]);


    //UARTCharPut(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_BASE], 'T');
    UARTIntRegister(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_BASE], UARTStdioIntHandler);
    IntPrioritySet(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_INT], 0b00100000);
    UARTStdioConfig(nxtUARTNum, baudRate, nxtSys->getClockFreq());
    UARTEchoSet(false);
}

void Nextion::setTimeoutUS(uint32_t us)
{
    nxtTimeoutUS = us;
}

void Nextion::sendCmd(const char* cmd)
{
    UARTFlushRx();
    UARTprintf("%s%s", cmd, nxtEndStr);
}

void Nextion::setTxt(const char* comp, const char* txt)
{
    UARTFlushRx();
    UARTprintf("%s.txt=\"%s\"%s", comp, txt, nxtEndStr);
}

void Nextion::setVal(const char* comp, uint32_t val)
{
    UARTFlushRx();
    UARTprintf("%s.val=%i%s", comp, val, nxtEndStr);
}

void Nextion::setPage(const char* page)
{
    UARTFlushRx();
    UARTprintf("page %s%s", page, nxtEndStr);
}

void Nextion::setPage(uint32_t page)
{
    UARTFlushRx();
    UARTprintf("page %i%s", page, nxtEndStr);
}

void Nextion::flushRx()
{
    UARTFlushRx();
}

void Nextion::disableStdio()
{
    UARTIntDisable(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_BASE], 0xFFFFFFFF);
}

void Nextion::enableStdio()
{
    UARTIntEnable(NXT_UART_MAPPING[nxtUARTNum][NXT_UART_BASE], UART_INT_RX | UART_INT_RT);
}

uint32_t Nextion::getUARTBase()
{
    return NXT_UART_MAPPING[nxtUARTNum][NXT_UART_BASE];
}

uint32_t Nextion::getBaudRate()
{
    return nxtBaudRate;
}

uint32_t Nextion::charsAvail()
{
    return UARTRxBytesAvail();
}

char Nextion::getChar()
{
    return UARTgetc();
}

uint32_t Nextion::peek(const char c)
{
    return UARTPeek(c);
}

void Nextion::printf(const char *pcString, ...)
{
    va_list vaArgP;

    //
    // Start the varargs processing.
    //
    va_start(vaArgP, pcString);

    UARTvprintf(pcString, vaArgP);

    //
    // We're finished with the varargs now.
    //
    va_end(vaArgP);
}

char* Nextion::getTxt(const char* comp)
{
    /*UARTFlushRx();
    UARTprintf("get %s.txt%s", comp, nxtEndStr);

    uint32_t time = nxtSys->getSystemTimeUS();

    for (uint32_t i = 0; i < nxtReadDataSize; i++)
    {
        nxtReadData[i] = 0;
    }

    uint32_t readSize = 0

    while (UARTRxBytesAvail() < 7)
    {
        if (nxtSys->getSystemTimeUS() - time > )
    }
    */
    return 0;
}

uint32_t Nextion::getVal(const char* comp)
{
    UARTFlushRx();
    UARTprintf("get %s.val%s", comp, nxtEndStr);

    uint32_t time = nxtSys->getSystemTimeUS();

    while (UARTRxBytesAvail() < 8)
    {
        if (nxtSys->getSystemTimeUS() - time > nxtTimeoutUS)
        {
            return receiveTimeoutVal;
        }
        if (UARTRxBytesAvail() && UARTPeek('\x71') != 0)
        {
            UARTgetc();
        }
    }

    char data[8];
    for (uint32_t i = 0; i < 8; i++)
    {
        data[i] = UARTgetc();
    }

    if (data[5] == 0xff && data[6] == 0xff && data[7] == 0xff)
    {
        return ((((uint32_t) data[4]) << 24) + (((uint32_t) data[3]) << 16) + (((uint32_t) data[2]) << 8) + (uint32_t) data[1]);
    }
    else
    {
        return receiveErrorVal;
    }
}
