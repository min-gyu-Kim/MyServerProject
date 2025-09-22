#pragma once

#include <memory.h>

namespace test {
class RingBuffer
{
  private:
    enum { BUFFER_SIZE = 1024 * 1024 };

  public:
    RingBuffer()
    {
        Reset();
    }
    ~RingBuffer()
    {
    }

    void Reset()
    {
        mFront = 0;
        mRear = 0;
    }

    char* GetBufferPtr()
    {
        return mBuffer;
    }
    char* GetFrontBufferPtr()
    {
        return mBuffer + mFront;
    }
    char* GetRearBufferPtr()
    {
        return mBuffer + mRear;
    }

    int GetDirectWriteSize() const
    {
        if (mFront <= mRear) {
            return BUFFER_SIZE - mRear - 1;
        }

        return mFront - mRear - 1;
    }

    int GetDIrectReadSize() const
    {
        if (mFront <= mRear) {
            return mRear - mFront;
        } else {
            return BUFFER_SIZE - mFront;
        }
    }

    int GetUsedSize() const
    {
        if (mFront <= mRear) {
            return mRear - mFront;
        } else {
            return BUFFER_SIZE - mFront + mRear;
        }
    }

    int GetRemainSize() const
    {
        return BUFFER_SIZE - 1 - GetUsedSize();
    }

    bool Write(const char* inputBuffer, int size)
    {
        int remainSize = GetRemainSize();
        if (remainSize < size) {
            return false;
        }

        int directWriteSize = GetDirectWriteSize();
        if (size <= directWriteSize) {
            memcpy(mBuffer + mRear, inputBuffer, size);
        } else {
            memcpy(mBuffer + mRear, inputBuffer, directWriteSize);
            memcpy(mBuffer, inputBuffer + directWriteSize, size - directWriteSize);
        }

        mRear += size;
        mRear %= BUFFER_SIZE;

        return true;
    }

    int Peek(char* readBuffer, int size)
    {
        const int usedSize = GetUsedSize();
        int peekSize;
        if (usedSize <= size) {
            peekSize = usedSize;
        } else {
            peekSize = size;
        }

        int directReadSize = GetDIrectReadSize();
        if (peekSize <= directReadSize) {
            memcpy(readBuffer, mBuffer + mFront, peekSize);
        } else {
            memcpy(readBuffer, mBuffer + mFront, directReadSize);
            memcpy(readBuffer + directReadSize, mBuffer, peekSize - directReadSize);
        }

        return peekSize;
    }

    int Read(char* readBuffer, int size)
    {
        int peekSize = Peek(readBuffer, size);
        mFront += peekSize;
        mFront %= BUFFER_SIZE;

        return peekSize;
    }

    void MoveReadOffset(int moveSize)
    {
        mFront += moveSize;
        mFront %= BUFFER_SIZE;
    }

    void MoveWriteOffset(int moveSize)
    {
        mRear += moveSize;
        mRear %= BUFFER_SIZE;
    }

  private:
    int mFront;
    int mRear;
    char mBuffer[BUFFER_SIZE];
};
} // namespace test