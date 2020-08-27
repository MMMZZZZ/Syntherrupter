/*
 * UART.h
 *
 *  Created on: 20.08.2020
 *      Author: Max Zuidberg
 */

#ifndef UART_H_
#define UART_H_


#include <ByteBuffer.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"              // Macros defining the memory map of the Tiva C Series device. This includes defines such as peripheral base address locations such as GPIO_PORTF_BASE.
#include "inc/hw_types.h"               // Defines common types and macros.
#include "inc/hw_gpio.h"                // Defines and Macros for GPIO hardware.
#include "inc/hw_uart.h"                // Defines and macros used when accessing the UART.
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"          // Mapping of peripherals to pins for all parts.
#include "driverlib/sysctl.h"           // Defines and macros for System Control API of DriverLib. This includes API functions such as SysCtlClockSet.
#include "driverlib/gpio.h"             // Defines and macros for GPIO API of DriverLib. This includes API functions such as GPIOPinWrite.
#include "driverlib/interrupt.h"        // Defines and macros for NVIC Controller (Interrupt) API of driverLib. This includes API functions such as IntEnable and IntPrioritySet.
#include "driverlib/uart.h"             // Defines and macros for UART API of driverLib.
#include "System.h"


extern System sys;


class UART
{
public:
    UART();
    virtual ~UART();
    void init(uint32_t port, uint32_t rxPin, uint32_t txPin, uint32_t baudRate, void (*rxISR)(void),
              uint32_t intPriority = DEFAULT_INT_PRIO, bool buffered = true);
    void init(uint32_t uartNum, uint32_t baudRate, void (*rxISR)(void),
              uint32_t intPriority = DEFAULT_INT_PRIO, bool buffered = true);
    void sendChar(uint8_t chr)
    {
        UARTCharPut(uartBase, chr);
    };
    void ISR()
    {
        // Read and clear the asserted interrupts
        UARTIntClear(uartBase, UARTIntStatus(uartBase, true));

        // Store all available chars in bigger buffer.
        while (UARTCharsAvail(uartBase))
        {
            buffer.add(UARTCharGet(uartBase));
        }
    };
    ByteBuffer buffer;
    static constexpr uint32_t DEFAULT_INT_PRIO = 42424242;

private:
    uint32_t uartNum  = 0;
    volatile uint32_t uartBase = 0;
    static constexpr uint32_t UART_SYSCTL_PERIPH      = 0;
    static constexpr uint32_t UART_BASE               = 1;
    static constexpr uint32_t UART_PORT_SYSCTL_PERIPH = 2;
    static constexpr uint32_t UART_PORT_BASE          = 3;
    static constexpr uint32_t UART_RX_PIN_CFG         = 4;
    static constexpr uint32_t UART_TX_PIN_CFG         = 5;
    static constexpr uint32_t UART_RX_PIN             = 6;
    static constexpr uint32_t UART_TX_PIN             = 7;
    static constexpr uint32_t UART_INT                = 8;
    const uint32_t UART_MAPPING[8][9] = {{SYSCTL_PERIPH_UART0, UART0_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART0},
                                         {SYSCTL_PERIPH_UART1, UART1_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTB_BASE, GPIO_PB0_U1RX, GPIO_PB1_U1TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART1},
                                         {SYSCTL_PERIPH_UART2, UART2_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA6_U2RX, GPIO_PA7_U2TX, GPIO_PIN_6, GPIO_PIN_7, INT_UART2},
                                         {SYSCTL_PERIPH_UART3, UART3_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA4_U3RX, GPIO_PA5_U3TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART3},
                                         {SYSCTL_PERIPH_UART4, UART4_BASE, SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, GPIO_PK0_U4RX, GPIO_PK1_U4TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART4},
                                         {SYSCTL_PERIPH_UART5, UART5_BASE, SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PC6_U5RX, GPIO_PC7_U5TX, GPIO_PIN_6, GPIO_PIN_7, INT_UART5},
                                         {SYSCTL_PERIPH_UART6, UART6_BASE, SYSCTL_PERIPH_GPIOP, GPIO_PORTP_BASE, GPIO_PP0_U6RX, GPIO_PP1_U6TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART6},
                                         {SYSCTL_PERIPH_UART7, UART7_BASE, SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PC4_U7RX, GPIO_PC5_U7TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART7}};
};

#endif /* UART_H_ */
