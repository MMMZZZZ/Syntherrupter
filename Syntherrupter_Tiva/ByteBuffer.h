/*
 * ByteBuffer.h
 *
 *  Created on: 20.08.2020
 *      Author: Max Zuidberg
 */

#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_


#include <stdint.h>
#include <stdbool.h>


class ByteBuffer
{
public:
    ByteBuffer();
    virtual ~ByteBuffer();
    void init(uint32_t size);
    void add(volatile uint8_t data)
    {
        buffer[writeIndex++] = data;
        if (writeIndex == readIndex)
        {
            readIndex = writeIndex + 1;
        }
        if (writeIndex >= size - 1)
        {
            if (writeIndex >= size)
            {
                writeIndex = 0;
                readIndex  = 1;
            }
            else
            {
                readIndex  = 0;
            }
        }
    };

    void remove(volatile uint32_t count = 1)
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
    };
    void flush()
    {
        readIndex = writeIndex;
    }
    uint32_t level()
    {
        if (readIndex <= writeIndex)
        {
            return writeIndex - readIndex;
        }
        else
        {
            return size - readIndex + writeIndex;
        }
    };
    uint8_t peek()
    {
        return buffer[readIndex];
    };
    uint8_t read()
    {
        uint8_t data = peek();
        remove();
        return data;
    };
private:
    volatile uint8_t* buffer;
    volatile uint32_t size = 0, readIndex = 0, writeIndex = 0;
};

#endif /* BYTEBUFFER_H_ */
