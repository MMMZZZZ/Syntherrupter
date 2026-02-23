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
#include "MIDI.h"
#include "Tone.h"



class Effect
{
public:
    struct LFO {
        float freq;
        float amp;
    };

    float frequency;
    float volume;
    float basePitch;
    float baseVolume;
    struct LFO pitchLFO;
    struct LFO volumeLFO;

    Effect(float basePitch, float baseVolume, float pitchLFOFreq, float pitchLFOAmp, float volumeLFOFreq, float volumeLFOAmp)
    {
        this->basePitch = basePitch;
        this->baseVolume = baseVolume;
        this->pitchLFO.amp = pitchLFOAmp;
        this->pitchLFO.freq = pitchLFOFreq;
        this->volumeLFO.amp = volumeLFOAmp;
        this->volumeLFO.freq = volumeLFOFreq;
    };
    virtual ~Effect();
    float getLFOVal(struct LFO& lfo)
    {
        return sinf(6.283185307179586e-6f * float(System::getSystemTimeUS()) * lfo.freq) * lfo.amp;
    }
    uint32_t updateFreq(float pitchOffset)
    {
        float pitch = basePitch + pitchOffset + getLFOVal(pitchLFO);
        frequency = MIDI::getFreq(pitch);
    };
    float updateVolume(float volumeOffset)
    {
        volume = Branchless::max(0.0f, Branchless::min(1.0f, baseVolume + volumeOffset + getLFOVal(volumeLFO)));
    };
};


class LSData
{
public:
    LSData();
    virtual ~LSData();

    // Default values; may be tweaked by LightSaber to differentiate the four lightsabers.
    Effect buzz = Effect(26.0f, 0.90f, 0.0f, 0.0f, 0.10f, 0.1f);
    Effect hum1 = Effect(42.0f, 0.70f, 0.0f, 0.0f, 0.13f, 0.1f);
    Effect hum2 = Effect(43.0f, 0.25f, 0.0f, 0.0f, 0.00f, 0.1f);

    uint32_t lastReceive = 0;
    uint32_t assignedCoils = 0;
    bool updated;
    float volume = 0.0f;
    float periodUS = 0.0f;
    float ax = 0.0f, ay = 0.0f, az = 0.0f,
          gx = 0.0f, gy = 0.0f, gz = 0.0f;
    void setData(float* data)
    {
        float check = data[0] + data[1] + data[2] + data[3] + data[4] + data[5];
        if (check != 0.0f)
        {
            lastReceive = System::getSystemTimeUS();
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
        /*
         * Great resource: http://www.dblondin.com/071807.html
         */

        //float gyro = sqrtf(gx * gx + gz * gz);
        float accel = sqrtf(ax * ax + ay * ay + az * az) - 1;
        float gyro  = (fabsf(gx) + fabsf(gz)) / 2.0f;

        /*
         * Original Code, partially reused
         */
        float volume = fmaxf(0.0f, fminf(1.0f, accel * 0.4f + gyro / 1000.0f - 0.01f));

        if (volume >= peakVol)
        {
            peakVol = volume;
        }
        peakVol = filtered(0, peakVol, 0.09);
        volume = filtered(volume, peakVol, 0.5);

        // slowVol = filtered(volume, slowVol, 0.5);

        // float frequency = exp2f((42.0f - 69.0f - 4*slowVol) / 12.0f) * 440.0f;

        // float periodUS = 1e6f / frequency;

        // if (periodUS != this->periodUS || volume != this->volume)
        // {
        //     this->periodUS = periodUS;
        //     this->volume   = volume;
        //     this->changed  = (1 << COIL_COUNT) - 1;
        // }
        /*
         * End of original code.
         */

        buzz.updateFreq(0.0f);
        buzz.updateVolume(-0.5 * volume);
        hum1.updateFreq(0.8f * volume);
        hum1.updateVolume(volume);
        hum2.updateFreq(0.0f);
        hum2.updateVolume(0.0f);

        updated = true;
    };

private:
    float peakVol = 0.0f;
    //float slowVol = 0.0f;
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
