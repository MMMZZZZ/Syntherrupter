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


template <class T, uint32_t size>
class Buffer
{
public:
    Buffer()
    {
        // Default Constructor
        for (uint32_t i = 0; i < size; i++)
        {
            buffer[i] = 0;
        }
    };
    virtual ~Buffer()
    {
        // Default Destructor
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
        /*
         * Branchless version of
         *   if (readIndex != writeIndex
         *   {
         *       readIndex = (readIndex + 1) % size;
         *   }
         */
        bool increment = (readIndex != writeIndex);
        readIndex = (readIndex + increment) % size;
    };
    void flush()
    {
        readIndex = writeIndex;
    }
    uint32_t level()
    {
        return (size - readIndex + writeIndex) % (size + 1);
    };
    uint32_t avail()
    {
        return (size - writeIndex + readIndex) % (size + 1);
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
    static constexpr auto sze = size;
private:
    volatile T buffer[size];
    volatile uint32_t readIndex = 0, writeIndex = 0;
};

#endif /* BUFFER_H_ */
