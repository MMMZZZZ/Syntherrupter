/*
 * Nextion.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
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

void Nextion::init(uint32_t uartNumber, uint32_t baudRate, uint32_t timeoutUS)
{
    this->baudRate = baudRate;
    this->timeoutUS = timeoutUS;

    // Nextion UART stdio setup
    this->UARTNum = uartNumber;

    SysCtlPeripheralEnable(UART_MAPPING[UARTNum][UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(UART_MAPPING[UARTNum][UART_PORT_SYSCTL_PERIPH]);
    while (!SysCtlPeripheralReady(UART_MAPPING[UARTNum][UART_PORT_SYSCTL_PERIPH]));
    GPIOPinConfigure(UART_MAPPING[UARTNum][UART_RX_PIN_CFG]);
    GPIOPinConfigure(UART_MAPPING[UARTNum][UART_TX_PIN_CFG]);
    GPIOPinTypeUART(UART_MAPPING[UARTNum][UART_PORT_BASE],
                    UART_MAPPING[UARTNum][UART_TX_PIN] |
                    UART_MAPPING[UARTNum][UART_RX_PIN]);


    //UARTCharPut(UART_MAPPING[nxtUARTNum][UART_BASE], 'T');
    UARTIntRegister(UART_MAPPING[UARTNum][UART_BASE], UARTStdioIntHandler);
    IntPrioritySet(UART_MAPPING[UARTNum][UART_INT], 0b01000000);
    UARTStdioConfig(UARTNum, baudRate, sys.getClockFreq());
    UARTEchoSet(false);
}

void Nextion::setTimeoutUS(uint32_t us)
{
    timeoutUS = us;
}

void Nextion::sendCmd(const char* cmd)
{
    UARTFlushRx();
    UARTprintf("%s%s", cmd, endStr);
}

void Nextion::setTxt(const char* comp, const char* txt)
{
    UARTFlushRx();
    UARTprintf("%s.txt=\"%s\"%s", comp, txt, endStr);
}

void Nextion::setVal(const char* comp, uint32_t val)
{
    UARTFlushRx();
    UARTprintf("%s.val=%i%s", comp, val, endStr);
}

void Nextion::setPage(const char* page)
{
    UARTFlushRx();
    UARTprintf("page %s%s", page, endStr);
}

void Nextion::setPage(uint32_t page)
{
    UARTFlushRx();
    UARTprintf("page %i%s", page, endStr);
}

void Nextion::flushRx()
{
    UARTFlushRx();
}

void Nextion::disableStdio()
{
    UARTIntDisable(UART_MAPPING[UARTNum][UART_BASE], 0xFFFFFFFF);
}

void Nextion::enableStdio()
{
    UARTIntEnable(UART_MAPPING[UARTNum][UART_BASE], UART_INT_RX | UART_INT_RT);
}

uint32_t Nextion::getUARTBase()
{
    return UART_MAPPING[UARTNum][UART_BASE];
}

uint32_t Nextion::getBaudRate()
{
    return baudRate;
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

    uint32_t time = sys.getSystemTimeUS();

    for (uint32_t i = 0; i < nxtReadDataSize; i++)
    {
        nxtReadData[i] = 0;
    }

    uint32_t readSize = 0

    while (UARTRxBytesAvail() < 7)
    {
        if (sys.getSystemTimeUS() - time > )
    }
    */
    return 0;
}

uint32_t Nextion::getVal(const char* comp)
{
    UARTFlushRx();
    UARTprintf("get %s.val%s", comp, endStr);

    uint32_t time = sys.getSystemTimeUS();

    while (UARTRxBytesAvail() < 8)
    {
        if (sys.getSystemTimeUS() - time > timeoutUS)
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
