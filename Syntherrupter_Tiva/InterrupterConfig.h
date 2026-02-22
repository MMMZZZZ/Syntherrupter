/*
 * InterrupterSettings.h
 *
 *  Created on: 28.04.2020
 *      Author: Max Zuidberg
 */

#ifndef INTERRUPTER_CONFIG_H_
#define INTERRUPTER_CONFIG_H_


#include "stdbool.h"
#include "stdint.h"


#define TIVA_FW_VERSION "v4.3.0-beta.4"

#ifdef COIL_COUNT_1
#define COIL_COUNT 1
#endif
#ifdef COIL_COUNT_2
#define COIL_COUNT 2
#endif
#ifdef COIL_COUNT_3
#define COIL_COUNT 3
#endif
#ifdef COIL_COUNT_4
#define COIL_COUNT 4
#endif
#ifdef COIL_COUNT_5
#define COIL_COUNT 5
#endif
#ifdef COIL_COUNT_6
#define COIL_COUNT 6
#endif
#ifdef TEABUG
#define COIL_COUNT 6
#endif

static constexpr uint32_t TONE_COUNT_SIMPLE = 1;
static constexpr uint32_t TONE_COUNT_LS     = 1;
static constexpr uint32_t TONE_COUNT_MIDI   = 16;
static constexpr uint32_t TONE_COUNT_TOTAL  = TONE_COUNT_SIMPLE + TONE_COUNT_LS + TONE_COUNT_MIDI;

static_assert(TONE_COUNT_TOTAL <= 32, "More than 32 tones total! U sure this makes sense?");


#endif /* INTERRUPTER_CONFIG_H_ */
