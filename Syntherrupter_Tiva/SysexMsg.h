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
};


#endif /* SYSEXMSG_H_ */
