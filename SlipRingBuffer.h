/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * Thundercracker firmware
 *
 * Copyright <c> 2012 Sifteo, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SLIPRING_BUFFER_H_
#define SLIPRING_BUFFER_H_

#include <assert.h>
#include <stdint.h>

#define ALWAYS_INLINE inline __attribute__((always_inline))
#define ASSERT(_x) assert(_x)
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
// Produces a 'size of array is negative' compile error when the assert fails
#define STATIC_ASSERT(_x) ((void)sizeof(char[1 - 2 * !(_x)]))

namespace Slip
{

static const unsigned END = 0300;     // indicates end of packet
static const unsigned ESC = 0333;     // indicates byte stuffing
static const unsigned ESC_END = 0334; // ESC ESC_END means END data byte
static const unsigned ESC_ESC = 0335; // ESC ESC_ESC means ESC data byte

} // namespace Slip

template < unsigned tSize >
class SlipRingBuffer
{

    uint8_t bytes[tSize];
    unsigned len;

public:
    SlipRingBuffer() : len(0)
    {
    }

    ALWAYS_INLINE void reset()
    {
        len = 0;
    }

    ALWAYS_INLINE bool isFull() const
    {
        return len == tSize;
    }

    ALWAYS_INLINE bool isEmpty() const
    {
        return len == 0;
    }

    ALWAYS_INLINE uint8_t *data()
    {
        return bytes;
    }

    ALWAYS_INLINE unsigned length() const
    {
        return len;
    }

    ALWAYS_INLINE unsigned bytesFree() const
    {
        return tSize - len;
    }

    ALWAYS_INLINE unsigned payloadLen() const
    {
        return (len == 0) ? 0 : len - 1;
    }

    ALWAYS_INLINE void append(uint8_t b)
    {
        ASSERT(!isFull());
        bytes[len++] = b;
    }

    ALWAYS_INLINE void append(const void *src, unsigned count)
    {
        ASSERT(bytesFree() >= count);
        memcpy(bytes + len, src, count);
        len += count;
    }

    void appendSlip(uint8_t b)
    {
        switch (b) {
        case Slip::END:
            append(Slip::ESC);
            append(Slip::ESC_END);
            break;

        case Slip::ESC:
            append(Slip::ESC);
            append(Slip::ESC_ESC);
            break;

        default:
            append(b);
        }
    }

    void appendSlip(const void *src, unsigned count)
    {
        const uint8_t *p = reinterpret_cast< const uint8_t * >(src);
        while (count--) {
            appendSlip(*p);
            p++;
        }
    }

    template < typename T >
    void appendItemSlip(const T &s)
    {
        const uint8_t *p = reinterpret_cast< const uint8_t * >(&s);
        unsigned sz = sizeof s;
        while (sz--) {
            appendSlip(*p);
            p++;
        }
    }

    ALWAYS_INLINE void delimitSlip()
    {
        append(Slip::END);
    }
};

#endif // SLIPRING_BUFFER_H_
