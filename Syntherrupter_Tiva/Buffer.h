/*
 * Buffer.h
 *
 *  Created on: 20.08.2020
 *      Author: Max Zuidberg
 */

#ifndef BUFFER_H_
#define BUFFER_H_


#include <stdint.h>
#include <stdbool.h>


template <class T>
class Buffer
{
public:
    Buffer()
    {
        // Default Constructor
    };
    virtual ~Buffer()
    {
        // Default Destructor
    };
    void init(uint32_t size)
    {
        this->size = size;
        buffer = new T[size];
        for (uint32_t i = 0; i < size; i++)
        {
            buffer[i] = 0;
        }
    };
    void add(T data)
    {
        buffer[writeIndex] = data;
        writeIndex = (writeIndex + 1) % size;
        if (writeIndex == readIndex)
        {
            readIndex = (readIndex + 1) % size;
        }
    };

    void remove()
    {
        if (readIndex != writeIndex)
        {
            readIndex = (readIndex + 1) % size;
        }
    };
    void flush()
    {
        readIndex = writeIndex;
    }
    uint32_t level()
    {
        return (size - readIndex + writeIndex) % size;
    };
    T peek()
    {
        return buffer[readIndex];
    };
    T read()
    {
        T data = peek();
        remove();
        return data;
    };
private:
    volatile T* buffer;
    volatile uint32_t size = 0, readIndex = 0, writeIndex = 0;
};

#endif /* BUFFER_H_ */
