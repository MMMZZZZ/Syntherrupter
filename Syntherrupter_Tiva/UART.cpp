/*
 * UART.cpp
 *
 *  Created on: 20.08.2020
 *      Author: Max Zuidberg
 */

#include <UART.h>


constexpr uint32_t UART::UART_MAPPING[8][9];


UART::UART()
{
    // TODO Auto-generated constructor stub

}

UART::~UART()
{
    // TODO Auto-generated destructor stub
}

void UART::init(uint32_t port, uint32_t rxPin, uint32_t txPin, uint32_t baudRate,
                void (*rxISR)(void), uint32_t intPriority)
{
    uint32_t num = 42;
    for (uint32_t i = 0; i < 8; i++)
    {
        if (   port  == UART_MAPPING[i][UART_PORT_BASE]
            && rxPin == UART_MAPPING[i][UART_RX_PIN]
            && txPin == UART_MAPPING[i][UART_TX_PIN])
        {
            num = i;
        }
    }
    if (num == 42)
    {
        System::error();
    }
    else
    {
        init(num, baudRate, rxISR, intPriority);
    }
}

void UART::init(uint32_t uartNum, uint32_t baudRate,
                void (*ISR)(void), uint32_t intPriority)
{
    // Enable MIDI receiving over the USB UART (selectable baud rate) and a separate MIDI UART (31250 fixed baud rate).
    this->uartNum = uartNum;
    uartBase = UART_MAPPING[uartNum][UART_BASE];
    SysCtlPeripheralEnable(UART_MAPPING[uartNum][UART_SYSCTL_PERIPH]);
    SysCtlPeripheralEnable(UART_MAPPING[uartNum][UART_PORT_SYSCTL_PERIPH]);
    SysCtlDelay(3);
    GPIOPinConfigure(UART_MAPPING[uartNum][UART_RX_PIN_CFG]);
    GPIOPinConfigure(UART_MAPPING[uartNum][UART_TX_PIN_CFG]);
    GPIOPinTypeUART(   UART_MAPPING[uartNum][UART_PORT_BASE],
                       UART_MAPPING[uartNum][UART_TX_PIN]
                     | UART_MAPPING[uartNum][UART_RX_PIN]);
    UARTConfigSetExpClk(uartBase, System::getClockFreq(), baudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFODisable(uartBase);
    UARTIntEnable(uartBase, UART_INT_RX | UART_INT_TX);
    UARTIntRegister(uartBase, ISR);
    if (intPriority != DEFAULT_INT_PRIO)
    {
        IntPrioritySet(UART_MAPPING[uartNum][UART_INT], intPriority);
    }
}

bool UART::write(uint8_t* buffer, uint32_t size, bool discard)
{
    if (!size)
    {
        return true;
    }
    auto available = txBuffer.avail();

    // only if enoughSpace is true something is transmitted. Meaning there's
    // either enough space or the part that doesn't fit gets discarded or
    // nothing is transmitted at all.
    bool enoughSpace = discard || (size <= available);
    if (!enoughSpace)
    {
        return false;
    }

    uint32_t limit = Branchless::min(size, txBuffer.avail());
    for (uint32_t i = 0; i < limit; i++)
    {
        txBuffer.add(buffer[i]);
    }
    if (!txEnabled)
    {
        auto level = txBuffer.level();
        if (level)
        {
            /*
             * Branchless version of:
             *   if (level > 1)
             *   {
             *       txEnabled = true;
             *       UARTIntEnable(uartBase, UART_INT_TX);
             *   }
             */
            txEnabled = (level > 1);
            UARTIntEnable(uartBase, UART_INT_TX * txEnabled);
            UARTCharPut(uartBase, txBuffer.read());
        }
    }
    return true;
}

void UART::ISR()
{
    // Read and clear the asserted interrupts
    uint32_t intStatus = UARTIntStatus(uartBase, true);
    UARTIntClear(uartBase, intStatus);

    // Store all available chars in bigger buffer.
    if (intStatus & UART_INT_RX)
    {
        while (UARTCharsAvail(uartBase))
        {
            rxBuffer.add(UARTCharGet(uartBase));
        }
    }
    // Transmit as long as bytes are available
    if (intStatus & UART_INT_TX)
    {
        UARTCharPut(uartBase, txBuffer.read());

        /*
         * Brancless version of:
         *   if(!txBuffer.level())
         *   {
         *       txEnabled = false;
         *       UARTIntDisabled(uartBase, UART_INT_TX);
         *   }
         */
        txEnabled = txBuffer.level();
        UARTIntDisable(uartBase, UART_INT_TX * (!txEnabled));
    }
}
