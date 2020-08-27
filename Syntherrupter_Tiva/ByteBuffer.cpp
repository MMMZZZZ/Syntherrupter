/*
 * ByteBuffer.cpp
 *
 *  Created on: 20.08.2020
 *      Author: Max Zuidberg
 */

#include <ByteBuffer.h>

ByteBuffer::ByteBuffer()
{
    // TODO Auto-generated constructor stub

}

ByteBuffer::~ByteBuffer()
{
    // TODO Auto-generated destructor stub
}

void ByteBuffer::init(uint32_t size)
{
    this->size = size;
    buffer = new uint8_t[size];
    for (uint32_t i = 0; i < size; i++)
    {
        buffer[i] = 0;
    }
}
