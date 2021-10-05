/*
 * Nextion.cpp
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */

#include <Nextion.h>


constexpr uint32_t Nextion::defaultTimeoutUS;


Nextion::Nextion()
{
    // TODO Auto-generated constructor stub

}

Nextion::~Nextion()
{
    // TODO Auto-generated destructor stub
}

void Nextion::init(uint32_t uartNumber, uint32_t baudRate)
{
    this->baudRate = baudRate;

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
    UARTStdioConfig(UARTNum, baudRate, System::getClockFreq());
    UARTEchoSet(false);

    // Try establishing a connection.
    // Try 3 times
    for (uint32_t i = 0; i < 3; i++)
    {
        // In case Nextion is already running
        acknowledgeEnabled = false;
        setVal("bkcmd", 0, NO_EXT);

        sendCmd("rest");

        // Nextion startup code: 00 00 00 FF FF FF
        if (!acknowledge(0x00, startTimeoutUS))
        {
            continue;
        }
        // Nextion ready code: 88 FF FF FF
        if (!acknowledge(0x88))
        {
            continue;
        }
        acknowledgeEnabled = true;
        if (setVal("bkcmd", 3, NO_EXT))
        {
            initOk = true;
            return;
        }
    }

    initOk = false;
}

bool Nextion::acknowledge(char code, uint32_t timeoutUS)
{
    uint32_t time = System::getSystemTimeUS();
    uint32_t termination = 0;
    int32_t response = -1;
    while (termination < 3)
    {
        if (System::getSystemTimeUS() - time > timeoutUS)
        {
            return false;
        }
        if (UARTRxBytesAvail())
        {
            int32_t temp = UARTgetc();
            if (temp == 0xff)
            {
                termination++;
            }
            else
            {
                response = temp;
            }
        }
    }

    return (response == code);
}

bool Nextion::sendCmd(const char* cmd)
{
    UARTFlushRx();
    UARTprintf(cmd);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::sendCmd(const char* cmd, const char* str)
{
    UARTFlushRx();
    UARTprintf(cmd, str);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::sendCmd(const char* cmd, int32_t val)
{
    UARTFlushRx();
    UARTprintf(cmd, val);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::sendCmd(const char* cmd, int32_t val1, int32_t val2)
{
    UARTFlushRx();
    UARTprintf(cmd, val1, val2);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::sendCmd(const char* cmd, const char* str, int32_t val)
{
    UARTFlushRx();
    UARTprintf(cmd, str, val);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::sendCmd(const char* cmd, int32_t val, const char* str)
{
    UARTFlushRx();
    UARTprintf(cmd, val, str);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::setTxt(const char* comp, const char* txt)
{
    UARTFlushRx();
    UARTprintf("%s.txt=\"%s\"", comp, txt);
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::setVal(const char* comp, uint32_t val, bool noExt)
{
    UARTFlushRx();
    if (noExt)
    {
        UARTprintf("%s=%i", comp, val);
    }
    else
    {
        UARTprintf("%s.val=%i", comp, val);
    }
    UARTwrite(endStr, 3);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::setPage(const char* page)
{
    UARTFlushRx();
    sendCmd("page %s", page);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

bool Nextion::setPage(uint32_t page)
{
    UARTFlushRx();
    sendCmd("page %i", page);
    if (acknowledgeEnabled)
    {
        return acknowledge();
    }
    return true;
}

/*void Nextion::flushRx()
{
    UARTFlushRx();
}*/

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

uint32_t Nextion::getUARTPeriph()
{
    return UART_MAPPING[UARTNum][UART_SYSCTL_PERIPH];
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

/*void Nextion::printf(const char *pcString, ...)
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
}*/

char* Nextion::getTxt(const char* comp)
{
    /*UARTFlushRx();
    UARTprintf("get %s.txt%s", comp, nxtEndStr);

    uint32_t time = System::getSystemTimeUS();

    for (uint32_t i = 0; i < nxtReadDataSize; i++)
    {
        nxtReadData[i] = 0;
    }

    uint32_t readSize = 0

    while (UARTRxBytesAvail() < 7)
    {
        if (System::getSystemTimeUS() - time > )
    }
    */
    return 0;
}

int32_t Nextion::getVal(const char* comp)
{
    UARTFlushRx();
    UARTprintf("get %s.val%s", comp, endStr);

    uint32_t time = System::getSystemTimeUS();

    while (UARTRxBytesAvail() < 8)
    {
        if (System::getSystemTimeUS() - time > defaultTimeoutUS)
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
