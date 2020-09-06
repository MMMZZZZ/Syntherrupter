/*
 * LSData.h
 *
 *  Created on: 06.09.2020
 *      Author: Max Zuidberg
 */


#ifndef LSDATA_H_
#define LSDATA_H_


#include <math.h>
#include "InterrupterConfig.h"
#include "System.h"
#include "Tone.h"


class LSData
{
public:
    LSData();
    virtual ~LSData();

    Tone* assignedTones[COIL_COUNT];

    uint32_t lastUpdate = 0;
    uint32_t assignedCoils = 0;
    uint32_t changed = 0;
    float volume = 0.0f;
    float periodUS = 0.0f;
    float ax = 0.0f, ay = 0.0f, az = 0.0f,
          gx = 0.0f, gy = 0.0f, gz = 0.0f;
    void setData(float* data)
    {
        lastUpdate = System::getSystemTimeUS();
        ax = data[0];
        ay = data[1];
        az = data[2];
        gx = data[3];
        gy = data[4];
        gz = data[5];
    };
    void process()
    {
        float periodUS = 1e6f / 90.0f;
        float volume   = fminf(100.0f, sqrtf(gx * gx + gy * gy + gz * gz)) / 100.0f;
        if (periodUS != this->periodUS || volume != this->volume)
        {
            this->periodUS = periodUS;
            this->volume   = volume;
            this->changed  = (1 << COIL_COUNT) - 1;
        }
    };
};


#endif /* LSDATA_H_ */
