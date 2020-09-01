/*
 * Tone.cpp
 *
 *  Created on: 16.08.2020
 *      Author: Max Zuidberg
 */


#include <Tone.h>
#include "ToneList.h"


Tone::Tone()
{
    srand(System::getSystemTimeUS());
}

Tone::~Tone()
{
    // TODO Auto-generated destructor stub
}

void Tone::remove(void* origin)
{
    if (origin == this->origin)
    {
        parent->deleteTone(this);
    }
}
