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

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_



 /**
  * Template for a statically-sized ring buffer.
  */
template <unsigned tSize, typename tItemType = uint8_t, typename tIndexType = uint16_t>
class RingBuffer
{
	tIndexType mHead;      /// Index of the next item to read
	tIndexType mTail;      /// Index of the next empty slot to write into
	tItemType mBuf[tSize];

public:
	RingBuffer() :
		mHead(0),
		mTail(0)
	{}

	void  init() {
		// must be power of 2
		STATIC_ASSERT((tSize & (tSize - 1)) == 0);
		mHead = mTail = 0;
	}

	unsigned  capacity() const {
		return tSize - 1;
	}

	void  enqueue(tItemType c)
	{
		ASSERT(!full());
		unsigned tail = mTail;
		mBuf[tail] = c;
		mTail = capacity() & (tail + 1);
	}

	tItemType  dequeue()
	{
		ASSERT(!empty());
		unsigned head = mHead;
		tItemType c = mBuf[head];
		mHead = capacity() & (head + 1);
		return c;
	}

	/*
	 * DMA support
	 */

	void fill(tItemType c)
	{
		for (tIndexType t = 0; t < tSize; ++t)
			mBuf[t] = c;
	}

	unsigned  getDMACount() const {
		return tSize;
	}

	uintptr_t  getDMABuffer() const {
		return (uintptr_t)& mBuf[0];
	}

	void  dequeueWithDMACount(unsigned count) {
		// Update the 'head' pointer, given an updated DMA count.
		ASSERT(count < getDMACount());
		mHead = capacity() & (tSize - count);
	}

	/*
	 * Copy from 'src' to 'this' until the source is empty or destination
	 * has >= fillThreshold items in the queue.
	 */
	template <typename T>
	void  pull(T& src, unsigned fillThreshold = tSize - 1)
	{
		ASSERT(fillThreshold <= capacity());
		unsigned rCount = src.readAvailable();
		unsigned selfReadAvailable = readAvailable();
		if (selfReadAvailable < fillThreshold) {
			unsigned wCount = fillThreshold - selfReadAvailable;
			unsigned count = MIN(rCount, wCount);
			while (count--) {
				enqueue(src.dequeue());
			}
		}
	}

	bool  full() const {
		return (capacity() & (mTail + 1)) == mHead;
	}

	bool  empty() const {
		return mHead == mTail;
	}

	unsigned  readAvailable() const {
		unsigned head = mHead;
		unsigned tail = mTail;
		return (tail - head) & capacity();
	}

	unsigned  writeAvailable() const {
		return capacity() - readAvailable();
	}
};

#endif // RING_BUFFER_H_
