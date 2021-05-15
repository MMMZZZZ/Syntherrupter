/*
 * Nextion.h
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
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


class Nextion
{
public:
    Nextion();
    virtual ~Nextion();
    bool init(uint32_t portNumber, uint32_t baudRate);
    bool sendCmd(const char* cmd);
    bool sendCmd(const char* cmd, const char* data);
    bool sendCmd(const char* cmd, int32_t val);
    bool sendCmd(const char* cmd, int32_t val1, int32_t val2);
    bool sendCmd(const char* cmd, const char* str, int32_t val);
    bool sendCmd(const char* cmd, int32_t val, const char* str);
    bool setTxt(const char* comp, const char* txt);
    bool setVal(const char* comp, uint32_t val, bool noExt = false);
    bool setPage(const char* page);
    bool setPage(uint32_t page);
    void flushRx();
    void printf(const char *pcString, ...);
    void disableStdio();
    void enableStdio();
    uint32_t getUARTBase();
    uint32_t getUARTPeriph();
    uint32_t getBaudRate();
    uint32_t charsAvail();
    uint32_t peek(const char c);
    char getChar();
    int32_t getVal(const char* comp);
    char* getTxt(const char* comp);
    static constexpr int32_t receiveErrorVal   = -24242420;
    static constexpr int32_t receiveTimeoutVal = -24242421;
    static constexpr uint32_t timeoutUS         =    300000;
    static constexpr uint32_t startTimeoutUS    =   3000000;
    static constexpr bool NO_EXT = true;
private:
    bool acknowledge();

    // UART mapping
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


    static constexpr char* endStr = "\xff\xff\xff";

    // Constants for communication with the nextion touch display
    static constexpr uint32_t TOUCH_EVENT_DATA    = 0x65;
    static constexpr uint32_t PAGE_ID             = 0x66;
    static constexpr uint32_t TOUCH_COORD         = 0x67;
    static constexpr uint32_t TOUCH_EVENT_SLEEP   = 0x68;
    static constexpr uint32_t STRING_DATA         = 0x70;
    static constexpr uint32_t INT_DATA            = 0x71;
    static constexpr uint32_t ENTER_SLEEP         = 0x86;
    static constexpr uint32_t WAKE_UP             = 0x87;
    static constexpr uint32_t STARTUP_OK          = 0x88;

    uint32_t UARTNum = 0;
    uint32_t baudRate = 0;
    static constexpr uint32_t readDataSize = 100;
    char readData[readDataSize];
    bool acknowledgeEnabled = false;
};

#endif /* NEXTION_H_ */
