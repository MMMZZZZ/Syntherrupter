/*
 * InterrupterSettings.h
 *
 *  Created on: 28.04.2020
 *      Author: Max Zuidberg
 */

#define TIVA_FW_VERSION "v3.1.1"

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
#ifdef DEBUG
#define COIL_COUNT 6
#endif

#define MAX_VOICES 16
