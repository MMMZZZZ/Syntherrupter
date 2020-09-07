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
        float check = data[0] + data[1] + data[2] + data[3] + data[4] + data[5];
        if (check != 0.0f)
        {
            lastUpdate = System::getSystemTimeUS();
            ax = data[0];
            ay = data[1];
            az = data[2];
            gx = data[3];
            gy = data[4];
            gz = data[5];
        }
    };
    void process()
    {
        float gyro = sqrtf(gx * gx + gz * gz);
        float accel = sqrtf(ax * ax + ay * ay + az * az) - 1;
        /*static float gyroFiltered{0};
        static float accelFiltered{0};

        gyroFiltered = filtered(gyro, gyroFiltered, 0.1f);
        accelFiltered = filtered(accel, accelFiltered, 0.1f);

        gyro -= gyroFiltered;
        accel -= accelFiltered;

        float volume   = fminf(1.0f, fmaxf(0.0f, gyro / 100.0f + accel / 1.0f));
        static float volumeFiltered{0};
        volumeFiltered = filtered(volume, volumeFiltered, 0.1f);
        volume = filtered(volume, volumeFiltered, 0.4f);

        static float ayFiltered{0.0f};
        ayFiltered = fminf(1.0f, fmaxf(-1.0f, filtered(ay, ayFiltered, 0.2f)));

        float frequency = exp2f((40.0f - 69.0f + ayFiltered) / 12.0f) * 440.0f;*/

        float volume = fmaxf(0.0f, fminf(1.0f, accel / 2.0f + gyro / 600.0f));

        static float peakVol{0};
        if (volume >= peakVol)
        {
            peakVol = volume;
        }
        peakVol = filtered(0, peakVol, 0.08);
        volume = ((filtered(volume, peakVol, 0.5)));

        float frequency = exp2f((40.0f - 69.0f + volume) / 12.0f) * 440.0f;

        float periodUS = 1e6f / frequency;

        if (periodUS != this->periodUS || volume != this->volume)
        {
            this->periodUS = periodUS;
            this->volume   = volume;
            this->changed  = (1 << COIL_COUNT) - 1;
        }
    };

private:
    float filtered(float a, float b, float fact)
    {
        return fact * a + (1 - fact) * b;
    };
/*#define ACCELEROMETER_SENSITIVITY 8192.0
#define GYROSCOPE_SENSITIVITY 65.536

#define M_PI 3.14159265359

#define dt 0.01                         // 10 ms sample rate!

void ComplementaryFilter(float *pitch, float *roll)
{
    float pitchAcc, rollAcc;

    // Integrate the gyroscope data -> int(angularSpeed) = angle
    *pitch += ((float)gyrData[0] / GYROSCOPE_SENSITIVITY) * dt; // Angle around the X-axis
    *roll -= ((float)gyrData[1] / GYROSCOPE_SENSITIVITY) * dt;    // Angle around the Y-axis

    // Compensate for drift with accelerometer data if !bullshit
    // Sensitivity = -2 to 2 G at 16Bit -> 2G = 32768 && 0.5G = 8192
    int forceMagnitudeApprox = abs(accData[0]) + abs(accData[1]) + abs(accData[2]);
    if (forceMagnitudeApprox > 8192 && forceMagnitudeApprox < 32768)
    {
    // Turning around the X axis results in a vector on the Y-axis
        pitchAcc = atan2f((float)accData[1], (float)accData[2]) * 180 / M_PI;
        *pitch = *pitch * 0.98 + pitchAcc * 0.02;

    // Turning around the Y axis results in a vector on the X-axis
        rollAcc = atan2f((float)accData[0], (float)accData[2]) * 180 / M_PI;
        *roll = *roll * 0.98 + rollAcc * 0.02;
    }
}*/
};


#endif /* LSDATA_H_ */
