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
    void add(uint8_t data);
    void remove(uint32_t count = 1);
    uint32_t level();
    uint8_t peek(uint32_t offset = 0);
    uint8_t read();
private:
    volatile uint8_t* buffer;
    volatile uint32_t size = 0, readIndex = 0, writeIndex = 0;
};

#endif /* BYTEBUFFER_H_ */
