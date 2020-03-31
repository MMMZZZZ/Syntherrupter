/*
 * Nextion.h
 *
 *  Created on: 26.03.2020
 *      Author: Max
 */

#ifndef NEXTION_H_
#define NEXTION_H_


#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
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
#include "uartstdio.h"
#include "System.h"


#ifndef UART_BUFFERED
#define UART_BUFFERED                   // creates buffer for the nextion uart // constant predefined
#endif

class Nextion
{
public:
    Nextion();
    virtual ~Nextion();
    void init(System* sys, uint32_t portNumber, uint32_t baudRate);
    void sendCmd(const char* cmd);
    void setTxt(const char* comp, const char* txt);
    void setVal(const char* comp, uint32_t val);
    void setPage(const char* page);
    void setPage(uint32_t page);
    void setTimeoutUS(uint32_t us);
    void flushRx();
    void printf(const char *pcString, ...);
    uint32_t charsAvail();
    char getChar();
    uint32_t getVal(const char* comp);
    char* getTxt(const char* comp);
    static constexpr uint32_t receiveErrorVal   = 424242420;
    static constexpr uint32_t receiveTimeoutVal = 424242421;

private:
    // UART mapping
    static constexpr uint32_t NXT_UART_SYSCTL_PERIPH      = 0;
    static constexpr uint32_t NXT_UART_BASE               = 1;
    static constexpr uint32_t NXT_UART_PORT_SYSCTL_PERIPH = 2;
    static constexpr uint32_t NXT_UART_PORT_BASE          = 3;
    static constexpr uint32_t NXT_UART_RX_PIN_CFG         = 4;
    static constexpr uint32_t NXT_UART_TX_PIN_CFG         = 5;
    static constexpr uint32_t NXT_UART_RX_PIN             = 6;
    static constexpr uint32_t NXT_UART_TX_PIN             = 7;
    static constexpr uint32_t NXT_UART_INT                = 8;
    const uint32_t NXT_UART_MAPPING[5][9] = {{SYSCTL_PERIPH_UART0, UART0_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART0},
                                             /*TBD*/{SYSCTL_PERIPH_UART1, UART1_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART1},
                                             /*TBD*/{SYSCTL_PERIPH_UART2, UART2_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA0_U0RX, GPIO_PA1_U0TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART2},
                                             {SYSCTL_PERIPH_UART3, UART3_BASE, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PA4_U3RX, GPIO_PA5_U3TX, GPIO_PIN_4, GPIO_PIN_5, INT_UART3},
                                             {SYSCTL_PERIPH_UART4, UART4_BASE, SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, GPIO_PK0_U4RX, GPIO_PK1_U4TX, GPIO_PIN_0, GPIO_PIN_1, INT_UART4}};


    static constexpr char* nxtEndStr = "\xff\xff\xff";

    // Constants for communication with the nextion touch display
    static constexpr uint32_t NXT_TOUCH_EVENT_DATA    = 0x65;
    static constexpr uint32_t NXT_PAGE_ID             = 0x66;
    static constexpr uint32_t NXT_TOUCH_COORD         = 0x67;
    static constexpr uint32_t NXT_TOUCH_EVENT_SLEEP   = 0x68;
    static constexpr uint32_t NXT_STRING_DATA         = 0x70;
    static constexpr uint32_t NXT_INT_DATA            = 0x71;
    static constexpr uint32_t NXT_ENTER_SLEEP         = 0x86;
    static constexpr uint32_t NXT_WAKE_UP             = 0x87;
    static constexpr uint32_t NXT_STARTUP_OK          = 0x88;

    System* nxtSys;
    uint32_t nxtTimeoutUS = 300000;
    uint32_t nxtUARTNum = 0;
    static constexpr uint32_t nxtReadDataSize = 100;
    char nxtReadData[nxtReadDataSize];
};

#endif /* NEXTION_H_ */
