/*
 * Pulse.h
 *
 *  Created on: 10.06.2021
 *      Author: Max
 */

#ifndef PULSE_H_
#define PULSE_H_


struct Pulse
{
    uint32_t timeUS;
    uint32_t ontimeUS;
};

static constexpr Pulse EMPTY_PULSE = {.timeUS = 0, .ontimeUS = 0};


#endif /* PULSE_H_ */
