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
}

void ByteBuffer::add(uint8_t data)
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

uint8_t ByteBuffer::peek(uint32_t offset)
{
    if (offset)
    {
        offset += readIndex;
        while (offset >= size)
        {
            offset -= size;
        }
        return buffer[offset];
    }
    return buffer[readIndex];
}

uint8_t ByteBuffer::read()
{
    uint32_t data = peek();
    if (level())
    {
        if (++readIndex >= size)
        {
            readIndex = 0;
        }
    }
    return data;
}

void ByteBuffer::remove(uint32_t count)
{
    readIndex += count;
    while (readIndex >= size)
    {
        readIndex -= size;
    }
    if (readIndex > writeIndex)
    {
        readIndex = writeIndex;
    }
}
