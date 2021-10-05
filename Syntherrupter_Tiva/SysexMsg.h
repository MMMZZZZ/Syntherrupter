/*
 * SysexMsg.h
 *
 *  Created on: 13.05.2021
 *      Author: Max Zuidberg
 */

#ifndef SYSEXMSG_H_
#define SYSEXMSG_H_


#include <stdint.h>
#include <stdbool.h>

class UART;

struct SysexMsg {
    uint32_t number;
    uint32_t targetLSB;
    uint32_t targetMSB;
    union {
        int32_t i32;
        uint32_t ui32;
        float f32;
        char chr[4];
    } value;
    uint8_t newMsg;
    UART* origin;
};


#endif /* SYSEXMSG_H_ */
