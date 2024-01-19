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
    UARTIntDisable(uartBase, UART_INT_TX);
    for (uint32_t i = 0; i < limit; i++)
    {
        txBuffer.add(buffer[i]);
    }
    if (txEnabled)
    {
        UARTIntEnable(uartBase, UART_INT_TX);
    }
    else
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
            UARTIntClear(uartBase, UART_INT_TX * txEnabled);
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

void UART::passthrough(uint32_t portANum, uint32_t portBNum)
{
    /*
     * Disable Interrupts and UARTs and pass data between the USB serial port
     * and the selected serial port.
     *
     * This is done via polling to operate independently of the baud rate
     * Since Syntherrupter does nothing else during this time, this doesn't
     * cause timing issues.
     *
     * Note: To leave this mode you have to power cycle Syntherrupter.
     */

    IntMasterDisable();

    uint32_t portA      =   UART::UART_MAPPING[portANum][UART::UART_PORT_BASE];
    uint32_t portARXPin =   UART::UART_MAPPING[portANum][UART::UART_RX_PIN];
    uint32_t portATXPin =   UART::UART_MAPPING[portANum][UART::UART_TX_PIN];
    uint32_t portB      =   UART::UART_MAPPING[portBNum][UART::UART_PORT_BASE];
    uint32_t portBRXPin =   UART::UART_MAPPING[portBNum][UART::UART_RX_PIN];
    uint32_t portBTXPin =   UART::UART_MAPPING[portBNum][UART::UART_TX_PIN];

    SysCtlPeripheralDisable(UART::UART_MAPPING[portANum][UART::UART_SYSCTL_PERIPH]);
    SysCtlPeripheralDisable(UART::UART_MAPPING[portBNum][UART::UART_SYSCTL_PERIPH]);

    GPIOPinTypeGPIOInput( portA, portARXPin);
    GPIOPinTypeGPIOInput( portB, portBRXPin);
    GPIOPinTypeGPIOOutput(portA, portATXPin);
    GPIOPinTypeGPIOOutput(portB, portBTXPin);
    GPIOPadConfigSet(     portA, portARXPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(     portB, portBRXPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


    while (42)
    {
        uint8_t portARXState = 0;
        uint8_t portBRXState = 0;

        // Read Pins
        // Branchless version of if(ARX): BTX=0xff else: BTX=0
        // GPIOPinRead(...) & 0xff required since upper 24 bits of the return value are not defined
        portARXState = ((bool) (GPIOPinRead(portA, portARXPin) & 0xff)) * (uint8_t) 0xff;
        portBRXState = ((bool) (GPIOPinRead(portB, portBRXPin) & 0xff)) * (uint8_t) 0xff;

        // Pass to other port
        GPIOPinWrite(portA, portATXPin, portBRXState);
        GPIOPinWrite(portB, portBTXPin, portARXState);
    }
}
