/*
 * GUI.h
 *
 *  Created on: 26.03.2020
 *      Author: Max Zuidberg
 */

#ifndef GUI_H_
#define GUI_H_


#include "stdint.h"
#include "stdbool.h"
#include "driverlib/gpio.h"
#include "System.h"
#include "InterrupterConfig.h"
#include "EEPROMSettings.h"
#include "Nextion.h"
#include "Coil.h"


class GUI
{
public:
    GUI();
    virtual ~GUI();
    static void init(Nextion* nextion, bool nxtOk);
    static uint32_t update();
    static void applyOutput();
    static void setError(const char* err);
    static void showError();
    static bool getAcceptsData()
    {
        return acceptsData;
    }
private:
    enum class Mode {
        emergency,
        idle,
        simpleEnter,
        simple,
        simpleExit,
        midiLiveEnter,
        midiLive,
        midiLiveExit,
        lightsaberEnter,
        lightsaber,
        lightsaberExit,
        userSelect,
        settings,
        settingsExit,
        nxtFWUpdate,
        espFWUpdate,
    };
    static void idle()
    {
        state = 0;
    };

    static void simpleEnter()
    {
        Simple::start();
    };

    static void simple();
    static void simpleExit()
    {
        Simple::stop();
    };

    static void midiLiveEnter()
    {
        MIDI::start();
    };

    static void midiLive();
    static void midiLiveExit()
    {
        // Stop MIDI operation
        MIDI::stop();
        EEE = false;
    };

    static void lightsaberEnter()
    {
        LightSaber::start();
    }
    static void lightsaber();
    static void lightsaberExit()
    {
        LightSaber::stop();
    }

    static void userSelect();
    static void settings();
    static void settingsExit()
    {
        // Update EEPROM
        EEPROMSettings::update();
    };

    static void serialPassthrough(uint32_t uartNum);

    static bool checkValue(int32_t val);

    static Nextion* nxt;

    static bool acceptsData;

    static uint32_t state;
    static uint32_t command;
    static uint32_t commandData[33]; // sized to max length of all commands.
    static Mode mode;

    static constexpr uint32_t errorLen = 20;
    static char errorTxt[errorLen];

    static bool EEE;
    static uint32_t EET;
    static uint32_t EEI;
    static constexpr uint32_t EES = 1572;
    static constexpr uint8_t EED[EES][4] = {{0, 0xb8, 0x79, 0x00}, {0, 0xb8, 0x7b, 0x00}, {0, 0xbd, 0x79, 0x00}, {0, 0xbd, 0x7b, 0x00}, {1, 0xb8, 0x00, 0x00}, {0  , 0xc8, 0x01, 0x00}, {0, 0xb8, 0x07, 0x64}, {0, 0xbd, 0x00, 0x00}, {0, 0xcd, 0x01, 0x00}, {0, 0xbd, 0x07, 0x64}, {0, 0x9d, 0x35, 0x6e}, {53, 0x8d, 0x35, 0x00}, {0, 0x9d, 0x3a, 0x6e}, {19, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x3f, 0x6e}, {112, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x3e, 0x6e}, {37, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x3a, 0x6e}, {26, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x37, 0x6e}, {25, 0x8d, 0x37, 0x00}, {0, 0x9d, 0x3c, 0x6e}, {25, 0x8d, 0x3c, 0x00}, {0, 0x9d, 0x41, 0x6e}, {0, 0x9d, 0x41, 0x7f}, {56, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x46, 0x7f}, {19, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x4b, 0x7f}, {112, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x4a, 0x7f}, {38, 0x8d, 0x4a, 0x00}, {0, 0x9d, 0x46, 0x7f}, {26, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x43, 0x7f}, {25, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x48, 0x7f}, {25, 0x8d, 0x48, 0x00}, {0, 0x9d, 0x4d, 0x7f}, {131, 0x8d, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x41, 0x7f}, {0, 0x9d, 0x4d, 0x7f}, {18, 0x8d, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x44, 0x7f}, {0, 0x9d, 0x50, 0x7f}, {125, 0x8d, 0x50, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x98, 0x50, 0x64}, {0, 0x98, 0x5c, 0x4a}, {6, 0x88, 0x5c, 0x00}, {0, 0x88, 0x50, 0x00}, {0, 0x88, 0x50, 0x00}, {0, 0x98, 0x4b, 0x64}, {0, 0x98, 0x4b, 0x64}, {0, 0x98, 0x57, 0x4a}, {5, 0x88, 0x57, 0x00}, {0, 0x88, 0x4b, 0x00}, {0, 0x88, 0x4b, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x98, 0x4d, 0x64}, {0, 0x98, 0x59, 0x4a}, {7, 0x88, 0x59, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x49, 0x64}, {0, 0x98, 0x49, 0x64}, {0, 0x98, 0x55, 0x4a}, {3, 0x88, 0x55, 0x00}, {0, 0x88, 0x49, 0x00}, {0, 0x88, 0x49, 0x00}, {0, 0x98, 0x4b, 0x64}, {0, 0x98, 0x4b, 0x64}, {0, 0x98, 0x57, 0x4a}, {5, 0x88, 0x57, 0x00}, {0, 0x88, 0x4b, 0x00}, {0, 0x88, 0x4b, 0x00}, {0, 0x98, 0x52, 0x4a}, {6, 0x88, 0x52, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x41, 0x7f}, {12, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {11, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x46, 0x7f}, {12, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x4a, 0x7f}, {13, 0x8d, 0x4a, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x48, 0x7f}, {10, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {12, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x88, 0x50, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {13, 0x8d, 0x3c, 0x00}, {0, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {11, 0x88, 0x50, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x9d, 0x44, 0x7f}, {11, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x4f, 0x7f}, {11, 0x8d, 0x4f, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x4d, 0x7f}, {13, 0x8d, 0x41, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {11, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3a, 0x64}, {11, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x98, 0x4b, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3c, 0x64}, {12, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x88, 0x4b, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3e, 0x64}, {11, 0x8d, 0x4d, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {12, 0x88, 0x50, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x46, 0x7f}, {12, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x4b, 0x7f}, {12, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x4d, 0x7f}, {13, 0x8d, 0x4d, 0x00}, {0, 0x9d, 0x4f, 0x7f}, {11, 0x8d, 0x4f, 0x00}, {0, 0x9d, 0x4b, 0x7f}, {12, 0x8d, 0x4b, 0x00}, {0, 0x98, 0x4a, 0x40}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x4d, 0x7f}, {36, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x4d, 0x00}, {0, 0x88, 0x4a, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x4b, 0x7f}, {11, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {12, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {11, 0x8d, 0x4b, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x4a, 0x7f}, {35, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x3a, 0x64}, {25, 0x8d, 0x4a, 0x00}, {0, 0x98, 0x4a, 0x40}, {0, 0x9d, 0x46, 0x7f}, {12, 0x88, 0x4a, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x48, 0x7f}, {11, 0x98, 0x4d, 0x64}, {11, 0x88, 0x4d, 0x00}, {0, 0x98, 0x51, 0x64}, {12, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x64}, {12, 0x88, 0x4f, 0x00}, {0, 0x98, 0x4b, 0x64}, {12, 0x88, 0x4b, 0x00}, {0, 0x98, 0x56, 0x64}, {11, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x88, 0x56, 0x00}, {0, 0x98, 0x54, 0x64}, {19, 0x88, 0x54, 0x00}, {0, 0x9d, 0x39, 0x7f}, {18, 0x8d, 0x39, 0x00}, {0, 0x9d, 0x3c, 0x7f}, {18, 0x8d, 0x3c, 0x00}, {0, 0x9d, 0x43, 0x7f}, {17, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x45, 0x7f}, {48, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x39, 0x7f}, {23, 0x8d, 0x39, 0x00}, {0, 0x9d, 0x42, 0x7f}, {37, 0x8d, 0x42, 0x00}, {10, 0x9d, 0x3e, 0x7f}, {13, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x47, 0x7f}, {11, 0x8d, 0x47, 0x00}, {0, 0x9d, 0x45, 0x7f}, {36, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x39, 0x7f}, {35, 0x8d, 0x39, 0x00}, {0, 0x98, 0x4e, 0x40}, {0, 0x9d, 0x42, 0x7f}, {6, 0x88, 0x4e, 0x00}, {0, 0x98, 0x4a, 0x40}, {7, 0x88, 0x4a, 0x00}, {0, 0x98, 0x47, 0x40}, {5, 0x88, 0x47, 0x00}, {0, 0x98, 0x42, 0x40}, {6, 0x88, 0x42, 0x00}, {0, 0x98, 0x47, 0x40}, {6, 0x88, 0x47, 0x00}, {0, 0x98, 0x4a, 0x40}, {6, 0x8d, 0x42, 0x00}, {0, 0x88, 0x4a, 0x00}, {0, 0x98, 0x4e, 0x40}, {6, 0x88, 0x4e, 0x00}, {0, 0x98, 0x4a, 0x40}, {8, 0x88, 0x4a, 0x00}, {0, 0x98, 0x45, 0x40}, {6, 0x88, 0x45, 0x00}, {0, 0x98, 0x4a, 0x40}, {7, 0x88, 0x4a, 0x00}, {0, 0x98, 0x4e, 0x40}, {8, 0x88, 0x4e, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x7f}, {35, 0x88, 0x51, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x47, 0x7f}, {0, 0x9d, 0x43, 0x64}, {35, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x48, 0x7f}, {36, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x9d, 0x47, 0x7f}, {0, 0x9d, 0x43, 0x64}, {35, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x45, 0x7f}, {11, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x3e, 0x7f}, {13, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x45, 0x7f}, {12, 0x8d, 0x45, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x43, 0x7f}, {11, 0x88, 0x4f, 0x00}, {0, 0x98, 0x4f, 0x40}, {11, 0x88, 0x4f, 0x00}, {0, 0x98, 0x4f, 0x40}, {12, 0x88, 0x4f, 0x00}, {0, 0x98, 0x4f, 0x40}, {12, 0x88, 0x4f, 0x00}, {0, 0x98, 0x45, 0x64}, {0, 0xb8, 0x5b, 0x7f}, {5, 0x98, 0x47, 0x64}, {3, 0x88, 0x45, 0x00}, {0, 0x98, 0x4a, 0x64}, {4, 0x88, 0x47, 0x00}, {0, 0x98, 0x4c, 0x64}, {4, 0x88, 0x4a, 0x00}, {0, 0x98, 0x4e, 0x64}, {4, 0x88, 0x4c, 0x00}, {0, 0x98, 0x4f, 0x64}, {4, 0x88, 0x4e, 0x00}, {0, 0x98, 0x51, 0x64}, {4, 0x88, 0x4f, 0x00}, {0, 0x98, 0x4e, 0x64}, {3, 0x88, 0x51, 0x00}, {1, 0x98, 0x4c, 0x64}, {4, 0x88, 0x4e, 0x00}, {0, 0x98, 0x4a, 0x64}, {3, 0x88, 0x4c, 0x00}, {1, 0x98, 0x47, 0x64}, {3, 0x88, 0x4a, 0x00}, {0, 0x98, 0x45, 0x64}, {5, 0x88, 0x47, 0x00}, {0, 0x98, 0x43, 0x64}, {3, 0x88, 0x45, 0x00}, {0, 0x98, 0x42, 0x64}, {4, 0x88, 0x43, 0x00}, {0, 0x98, 0x40, 0x64}, {4, 0x88, 0x42, 0x00}, {0, 0x88, 0x40, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0xb8, 0x5b, 0x40}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x41, 0x7f}, {12, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x8d, 0x41, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {13, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x9d, 0x46, 0x7f}, {11, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x4a, 0x7f}, {12, 0x8d, 0x4a, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x48, 0x7f}, {12, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {10, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {12, 0x88, 0x50, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x44, 0x64}, {11, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x44, 0x7f}, {12, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x4f, 0x7f}, {12, 0x8d, 0x4f, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x4d, 0x7f}, {12, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {11, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x8d, 0x3a, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3a, 0x64}, {12, 0x8d, 0x3e, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x98, 0x4b, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3c, 0x64}, {12, 0x88, 0x4b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x98, 0x4d, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3e, 0x64}, {12, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x4d, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {13, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x46, 0x7f}, {10, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x4b, 0x7f}, {12, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x4d, 0x7f}, {12, 0x8d, 0x4d, 0x00}, {0, 0x9d, 0x4f, 0x7f}, {12, 0x8d, 0x4f, 0x00}, {0, 0x9d, 0x4b, 0x7f}, {12, 0x8d, 0x4b, 0x00}, {0, 0x98, 0x4a, 0x40}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x4d, 0x7f}, {35, 0x8d, 0x4d, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x88, 0x4a, 0x00}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x4b, 0x7f}, {36, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x4a, 0x7f}, {35, 0x8d, 0x4a, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x4a, 0x7f}, {12, 0x8d, 0x4a, 0x00}, {0, 0x9d, 0x46, 0x7f}, {12, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x4a, 0x7f}, {11, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x35, 0x7f}, {0, 0x9d, 0x48, 0x7f}, {12, 0x8d, 0x35, 0x00}, {0, 0x9d, 0x37, 0x7f}, {11, 0x8d, 0x37, 0x00}, {0, 0x9d, 0x39, 0x7f}, {12, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x98, 0x4d, 0x40}, {0, 0x9d, 0x3c, 0x7f}, {14, 0x8d, 0x3c, 0x00}, {0, 0x9d, 0x3b, 0x7f}, {11, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x39, 0x7f}, {12, 0x88, 0x4d, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x98, 0x50, 0x40}, {0, 0x9d, 0x38, 0x7f}, {0, 0x9d, 0x4b, 0x58}, {12, 0x8d, 0x38, 0x00}, {0, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x3a, 0x7f}, {0, 0x9d, 0x4b, 0x58}, {11, 0x8d, 0x4b, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x3c, 0x7f}, {0, 0x9d, 0x4b, 0x58}, {11, 0x88, 0x50, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x4b, 0x00}, {0, 0x98, 0x4e, 0x64}, {0, 0x9d, 0x3f, 0x7f}, {0, 0x9d, 0x4b, 0x58}, {5, 0x88, 0x4e, 0x00}, {0, 0x98, 0x50, 0x64}, {6, 0x88, 0x50, 0x00}, {0, 0x98, 0x52, 0x64}, {1, 0x8d, 0x4b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x3d, 0x7f}, {0, 0x9d, 0x4b, 0x58}, {3, 0x88, 0x52, 0x00}, {0, 0x98, 0x54, 0x64}, {6, 0x88, 0x54, 0x00}, {0, 0x98, 0x56, 0x64}, {3, 0x8d, 0x4b, 0x00}, {0, 0x8d, 0x3d, 0x00}, {0, 0x9d, 0x3c, 0x7f}, {0, 0x9d, 0x4b, 0x58}, {2, 0x88, 0x56, 0x00}, {0, 0x98, 0x57, 0x64}, {6, 0x88, 0x57, 0x00}, {0, 0x98, 0x59, 0x64}, {4, 0x88, 0x59, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x4b, 0x00}, {0, 0x98, 0x5a, 0x64}, {0, 0x9d, 0x42, 0x40}, {0, 0x9d, 0x3f, 0x40}, {0, 0x9d, 0x3b, 0x40}, {13, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {11, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x88, 0x5a, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x98, 0x4e, 0x64}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {11, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x88, 0x4e, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x98, 0x57, 0x64}, {0, 0x9d, 0x44, 0x40}, {0, 0x9d, 0x3f, 0x40}, {0, 0x9d, 0x3b, 0x40}, {13, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {11, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {13, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x88, 0x57, 0x00}, {0, 0x98, 0x53, 0x64}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {10, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x88, 0x53, 0x00}, {0, 0x98, 0x5c, 0x64}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x88, 0x5c, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x98, 0x5a, 0x64}, {0, 0x9d, 0x42, 0x40}, {0, 0x9d, 0x3f, 0x40}, {0, 0x9d, 0x3b, 0x40}, {12, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x88, 0x5a, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x98, 0x4e, 0x64}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {11, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x88, 0x4e, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x98, 0x57, 0x64}, {0, 0x9d, 0x44, 0x40}, {0, 0x9d, 0x3f, 0x40}, {0, 0x9d, 0x3b, 0x40}, {11, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x8d, 0x44, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x44, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3b, 0x64}, {11, 0x88, 0x57, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x44, 0x00}, {0, 0x98, 0x4e, 0x64}, {0, 0x9d, 0x42, 0x7f}, {0, 0x9d, 0x42, 0x64}, {18, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x3d, 0x7f}, {6, 0x88, 0x4e, 0x00}, {0, 0x8d, 0x42, 0x00}, {12, 0x8d, 0x3d, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x3f, 0x7f}, {0, 0x9d, 0x44, 0x64}, {18, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x44, 0x7f}, {6, 0x88, 0x50, 0x00}, {0, 0x8d, 0x44, 0x00}, {13, 0x8d, 0x44, 0x00}, {0, 0x98, 0x51, 0x64}, {0, 0x9d, 0x45, 0x7f}, {0, 0x9d, 0x45, 0x64}, {17, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x3d, 0x7f}, {7, 0x88, 0x51, 0x00}, {0, 0x8d, 0x45, 0x00}, {11, 0x8d, 0x3d, 0x00}, {0, 0x98, 0x50, 0x64}, {0, 0x9d, 0x40, 0x7f}, {0, 0x9d, 0x44, 0x64}, {17, 0x8d, 0x40, 0x00}, {0, 0x9d, 0x44, 0x7f}, {6, 0x88, 0x50, 0x00}, {0, 0x8d, 0x44, 0x00}, {12, 0x8d, 0x44, 0x00}, {0, 0x98, 0x4e, 0x64}, {0, 0x9d, 0x42, 0x7f}, {0, 0x9d, 0x42, 0x64}, {12, 0x88, 0x4e, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x3b, 0x7f}, {11, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x42, 0x7f}, {12, 0x8d, 0x42, 0x00}, {0, 0x98, 0x4c, 0x64}, {0, 0x9d, 0x40, 0x7f}, {0, 0x9d, 0x40, 0x64}, {53, 0x88, 0x4c, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x98, 0x49, 0x64}, {0, 0x9d, 0x3d, 0x7f}, {0, 0x9d, 0x3d, 0x64}, {0, 0x9d, 0x45, 0x64}, {19, 0x88, 0x49, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x3d, 0x00}, {0, 0x8d, 0x3d, 0x00}, {0, 0x98, 0x4e, 0x64}, {0, 0x9d, 0x42, 0x7f}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x39, 0x64}, {4, 0x88, 0x4e, 0x00}, {0, 0x98, 0x4f, 0x64}, {5, 0x88, 0x4f, 0x00}, {0, 0x98, 0x51, 0x64}, {5, 0x88, 0x51, 0x00}, {0, 0x98, 0x53, 0x64}, {3, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x42, 0x00}, {3, 0x88, 0x53, 0x00}, {0, 0x98, 0x54, 0x64}, {4, 0x88, 0x54, 0x00}, {0, 0x98, 0x56, 0x64}, {6, 0x88, 0x56, 0x00}, {0, 0x98, 0x57, 0x64}, {5, 0x88, 0x57, 0x00}, {0, 0x98, 0x58, 0x64}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x4c, 0x64}, {0, 0x9d, 0x4f, 0x64}, {11, 0x8d, 0x4f, 0x00}, {0, 0x8d, 0x4c, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {12, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {13, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {11, 0x88, 0x58, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x98, 0x54, 0x64}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {12, 0x88, 0x54, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x98, 0x55, 0x64}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {12, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x88, 0x55, 0x00}, {0, 0x98, 0x56, 0x64}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x4e, 0x64}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x45, 0x64}, {11, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x4e, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {11, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x88, 0x56, 0x00}, {0, 0x98, 0x54, 0x64}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x8d, 0x42, 0x00}, {0, 0x88, 0x54, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x98, 0x5a, 0x64}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x88, 0x5a, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x98, 0x58, 0x64}, {0, 0x9d, 0x4f, 0x64}, {0, 0x9d, 0x4c, 0x64}, {0, 0x9d, 0x48, 0x64}, {12, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x4c, 0x00}, {0, 0x8d, 0x4f, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {11, 0x88, 0x58, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x98, 0x54, 0x64}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x40, 0x64}, {6, 0x88, 0x54, 0x00}, {0, 0x98, 0x55, 0x64}, {7, 0x88, 0x55, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x56, 0x64}, {0, 0x9d, 0x4e, 0x64}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x8d, 0x4e, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {11, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x42, 0x64}, {0, 0x9d, 0x45, 0x64}, {13, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x42, 0x7f}, {0, 0x9d, 0x45, 0x7f}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x45, 0x64}, {0, 0x9d, 0x42, 0x64}, {11, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x45, 0x7f}, {0, 0x9d, 0x48, 0x7f}, {12, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x9d, 0x42, 0x7f}, {0, 0x9d, 0x45, 0x7f}, {11, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x45, 0x7f}, {0, 0x9d, 0x48, 0x7f}, {12, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x9d, 0x42, 0x7f}, {0, 0x9d, 0x45, 0x7f}, {11, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x9d, 0x3e, 0x7f}, {0, 0x9d, 0x42, 0x7f}, {12, 0x88, 0x56, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x98, 0x43, 0x64}, {0, 0x98, 0x43, 0x64}, {0, 0x98, 0x58, 0x64}, {0, 0x9d, 0x40, 0x7f}, {0, 0x9d, 0x3c, 0x70}, {0, 0x9d, 0x4c, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x43, 0x64}, {13, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x4c, 0x00}, {0, 0x88, 0x58, 0x00}, {0, 0x88, 0x43, 0x00}, {0, 0x88, 0x43, 0x00}, {5, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x9d, 0x40, 0x7f}, {18, 0x8d, 0x40, 0x00}, {0, 0x98, 0x45, 0x64}, {0, 0x98, 0x45, 0x64}, {0, 0x98, 0x5a, 0x64}, {0, 0x98, 0x4a, 0x64}, {0, 0x9d, 0x3e, 0x7f}, {0, 0x9d, 0x4e, 0x64}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x88, 0x4a, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x8d, 0x4e, 0x00}, {0, 0x88, 0x5a, 0x00}, {0, 0x88, 0x45, 0x00}, {0, 0x88, 0x45, 0x00}, {6, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x45, 0x7f}, {17, 0x8d, 0x45, 0x00}, {0, 0x98, 0x5b, 0x64}, {0, 0x98, 0x4b, 0x64}, {0, 0x9d, 0x46, 0x7f}, {0, 0x9d, 0x4f, 0x64}, {0, 0x9d, 0x4b, 0x64}, {0, 0x9d, 0x46, 0x64}, {12, 0x88, 0x5b, 0x00}, {0, 0x8d, 0x46, 0x00}, {0, 0x8d, 0x4f, 0x00}, {0, 0x8d, 0x4b, 0x00}, {0, 0x88, 0x4b, 0x00}, {6, 0x8d, 0x46, 0x00}, {0, 0x9d, 0x3f, 0x7f}, {18, 0x8d, 0x3f, 0x00}, {0, 0x98, 0x59, 0x64}, {0, 0x98, 0x4a, 0x64}, {0, 0x9d, 0x3e, 0x7f}, {0, 0x9d, 0x4d, 0x64}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x46, 0x64}, {12, 0x8d, 0x4a, 0x00}, {0, 0x88, 0x4a, 0x00}, {0, 0x8d, 0x4d, 0x00}, {0, 0x8d, 0x46, 0x00}, {0, 0x88, 0x59, 0x00}, {6, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x41, 0x7f}, {17, 0x8d, 0x41, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x59, 0x64}, {0, 0x9d, 0x41, 0x7f}, {0, 0x9d, 0x43, 0x40}, {0, 0x9d, 0x4d, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x45, 0x64}, {6, 0x8d, 0x43, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {6, 0x8d, 0x41, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x59, 0x00}, {0, 0x8d, 0x4d, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x40, 0x7f}, {0, 0x9d, 0x43, 0x40}, {5, 0x8d, 0x43, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {6, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x3c, 0x7f}, {0, 0x9d, 0x43, 0x40}, {6, 0x88, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {7, 0x8d, 0x45, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x56, 0x64}, {0, 0x9d, 0x3e, 0x7f}, {0, 0x9d, 0x43, 0x40}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x43, 0x64}, {5, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {6, 0x8d, 0x45, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x43, 0x40}, {7, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {5, 0x8d, 0x45, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x43, 0x40}, {6, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {6, 0x8d, 0x45, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x43, 0x40}, {5, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {6, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x43, 0x40}, {6, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x88, 0x56, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x56, 0x64}, {0, 0x9d, 0x39, 0x7f}, {0, 0x9d, 0x45, 0x40}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x43, 0x64}, {6, 0x88, 0x51, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x40}, {0, 0x98, 0x4f, 0x40}, {0, 0x9d, 0x43, 0x40}, {6, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x51, 0x40}, {0, 0x98, 0x51, 0x40}, {0, 0x9d, 0x45, 0x40}, {6, 0x88, 0x51, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x88, 0x56, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x98, 0x4e, 0x40}, {0, 0x98, 0x4e, 0x40}, {0, 0x98, 0x56, 0x64}, {0, 0x98, 0x45, 0x64}, {0, 0x9d, 0x3e, 0x7f}, {0, 0x9d, 0x42, 0x40}, {0, 0x9d, 0x4a, 0x64}, {0, 0x9d, 0x45, 0x64}, {0, 0x9d, 0x42, 0x64}, {5, 0x88, 0x56, 0x00}, {0, 0x98, 0x55, 0x64}, {6, 0x88, 0x55, 0x00}, {0, 0x98, 0x53, 0x64}, {2, 0x88, 0x4e, 0x00}, {0, 0x88, 0x4e, 0x00}, {0, 0x8d, 0x42, 0x00}, {0, 0x8d, 0x3e, 0x00}, {2, 0x88, 0x53, 0x00}, {0, 0x98, 0x51, 0x64}, {3, 0x88, 0x45, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x4a, 0x00}, {0, 0x8d, 0x42, 0x00}, {2, 0x88, 0x51, 0x00}, {0, 0x98, 0x4f, 0x64}, {5, 0x88, 0x4f, 0x00}, {0, 0x98, 0x4e, 0x64}, {6, 0x88, 0x4e, 0x00}, {0, 0x98, 0x4c, 0x64}, {6, 0x88, 0x4c, 0x00}, {0, 0x98, 0x4a, 0x30}, {0, 0x98, 0x4a, 0x30}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {11, 0x8d, 0x47, 0x00}, {0, 0x88, 0x4a, 0x00}, {0, 0x88, 0x4a, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {11, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {12, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {12, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x98, 0x4f, 0x30}, {0, 0x98, 0x4f, 0x30}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {11, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x88, 0x4f, 0x00}, {0, 0x98, 0x53, 0x30}, {0, 0x98, 0x53, 0x30}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {12, 0x88, 0x53, 0x00}, {0, 0x88, 0x53, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x98, 0x51, 0x30}, {0, 0x98, 0x51, 0x30}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x88, 0x51, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x88, 0x51, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x45, 0x64}, {13, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x45, 0x64}, {10, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x45, 0x64}, {12, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x98, 0x4d, 0x30}, {0, 0x98, 0x4d, 0x30}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {12, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x88, 0x4d, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {12, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x98, 0x56, 0x30}, {0, 0x98, 0x56, 0x30}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {13, 0x88, 0x56, 0x00}, {0, 0x88, 0x56, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {12, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {11, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {11, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x45, 0x64}, {0, 0x9d, 0x41, 0x64}, {12, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x47, 0x64}, {0, 0x9d, 0x43, 0x64}, {12, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x47, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x9d, 0x48, 0x64}, {0, 0x9d, 0x45, 0x64}, {11, 0x8d, 0x45, 0x00}, {0, 0x8d, 0x48, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {12, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {11, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {13, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {12, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x39, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x39, 0x64}, {11, 0x8d, 0x39, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x4a, 0x7f}, {6, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x3b, 0x00}, {6, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {6, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {6, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x3e, 0x00}, {0, 0x9d, 0x3e, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {6, 0x8d, 0x3e, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x3b, 0x00}, {6, 0x8d, 0x4a, 0x00}, {0, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x4c, 0x7f}, {6, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x40, 0x00}, {7, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {5, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x3b, 0x00}, {0, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {6, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x9d, 0x40, 0x64}, {0, 0x9d, 0x3b, 0x64}, {0, 0x9d, 0x37, 0x64}, {6, 0x8d, 0x3b, 0x00}, {0, 0x8d, 0x40, 0x00}, {0, 0x8d, 0x37, 0x00}, {6, 0x8d, 0x4c, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x35, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x4d, 0x7f}, {6, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x35, 0x00}, {0, 0x8d, 0x41, 0x00}, {5, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x35, 0x64}, {0, 0x9d, 0x3a, 0x64}, {6, 0x8d, 0x35, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x35, 0x64}, {0, 0x9d, 0x3a, 0x64}, {6, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x35, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x35, 0x64}, {0, 0x9d, 0x3a, 0x64}, {6, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x35, 0x00}, {6, 0x8d, 0x4d, 0x00}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x38, 0x64}, {0, 0x9d, 0x4b, 0x7f}, {6, 0x8d, 0x38, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x3f, 0x00}, {6, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x38, 0x64}, {6, 0x8d, 0x38, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x38, 0x64}, {6, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x38, 0x00}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x38, 0x64}, {7, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x38, 0x00}, {0, 0x8d, 0x3f, 0x00}, {5, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x4f, 0x7f}, {35, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3a, 0x00}, {12, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x4f, 0x7f}, {12, 0x8d, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x3f, 0x64}, {0, 0x9d, 0x3a, 0x64}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x4f, 0x7f}, {12, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x3f, 0x00}, {0, 0x8d, 0x3a, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x3c, 0x64}, {0, 0x9d, 0x41, 0x64}, {0, 0x9d, 0x4f, 0x7f}, {24, 0x8d, 0x3c, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x41, 0x00}, {0, 0x8d, 0x4f, 0x00}, {0, 0x9d, 0x4d, 0x7f}, {22, 0x8d, 0x4d, 0x00}, {0, 0x9d, 0x4b, 0x7f}, {23, 0x8d, 0x4b, 0x00}, {0, 0x9d, 0x43, 0x64}, {0, 0x9d, 0x4f, 0x7f}, {24, 0x8d, 0x4f, 0x00}, {0, 0x8d, 0x43, 0x00}, {11, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x43, 0x7f}, {12, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x43, 0x7f}, {11, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x43, 0x7f}, {12, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x37, 0x00}, {0, 0x9d, 0x37, 0x64}, {0, 0x9d, 0x43, 0x7f}, {12, 0x8d, 0x43, 0x00}, {0, 0x8d, 0x37, 0x00}, {246, 0xb8, 0x79, 0x00}, {0, 0xb8, 0x7b, 0x00}, {0, 0xbd, 0x79, 0x00}, {0, 0xbd, 0x7b, 0x00}};
};

#endif /* GUI_H_ */
