/*
 * Channel.h
 *
 *  Created on: 10.05.2020
 *      Author: Max
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_


#include <stdbool.h>
#include <stdint.h>


class Channel
{
public:
    Channel();
    virtual ~Channel();
    volatile uint32_t volume = 0;
    volatile uint32_t modulation = 0;
    volatile uint32_t channelAfterTouch = 0;
    volatile uint32_t program = 127;
    volatile uint32_t coils = 0;
    volatile int32_t pitchBend = 0;
    volatile bool sustainPedal = false;
    volatile bool damperPedal  = false;
};

#endif /* CHANNEL_H_ */
