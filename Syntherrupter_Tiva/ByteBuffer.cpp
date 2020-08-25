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

void ByteBuffer::add(volatile uint8_t data)
{
    buffer[writeIndex++] = data;
    if (writeIndex >= size)
    {
        writeIndex = 0;
    }
    if (writeIndex == readIndex)
    {
        readIndex = writeIndex + 1;
    }
}

uint32_t ByteBuffer::level()
{
    if (readIndex <= writeIndex)
    {
        return writeIndex - readIndex;
    }
    else
    {
        return size - readIndex + writeIndex;
    }
}

uint8_t ByteBuffer::peek()
{
    return buffer[readIndex];
}

uint8_t ByteBuffer::read()
{
    uint8_t data = peek();
    remove();
    return data;
}

void ByteBuffer::remove(volatile uint32_t count)
{
    readIndex += count;
    if (readIndex >= size)
    {
        readIndex -= size;
    }
    if (readIndex > writeIndex)
    {
        readIndex = writeIndex;
    }
}
