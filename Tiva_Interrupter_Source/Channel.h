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
    uint32_t channelAfterTouch = 0;
    uint32_t program = 127;
    uint32_t coils = 0;
    int32_t pitchBend = 0;
    float volume = 0.0f;
    float modulation = 0.0f;
    bool sustainPedal = false;
    bool damperPedal  = false;
};

#endif /* CHANNEL_H_ */
