/*
 * Note.cpp
 *
 *  Created on: 08.05.2020
 *      Author: Max Zuidberg
 */

#include <Note.h>

Note::Note()
{
    // TODO Auto-generated constructor stub
    for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
    {
        panVol[coil] = 1.0f;
    }
}

Note::~Note()
{
    // TODO Auto-generated destructor stub
}
