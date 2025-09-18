#pragma once

#include "core/Types.hpp"

#include <memory.h>

namespace core{
class RecvBuffer{
private:
	enum { BUFFER_SIZE = 1024 * 1024 };
public:
	RecvBuffer()
	{

	}
	~RecvBuffer()
	{
		
	}

	void Reset()
	{
		mFront = 0;
		mRear = 0;
	}

	Byte* GetBufferPtr() { return mBuffer; }
	Byte* GetFrontBufferPtr() { return mBuffer + mFront; }
	Byte* GetRearBufferPtr() { return mBuffer + mRear; }

	Int32 GetDirectWriteSize() const
	{
		if(mFront <= mRear)
		{
			return BUFFER_SIZE - mRear - 1;
		}
		
		return mFront - mRear - 1;
	}

	Int32 GetDIrectReadSize() const
	{
		if (mFront <= mRear)
		{
			return mRear - mFront;
		}
		else
		{
			return BUFFER_SIZE - mFront;
		}
	}

	Int32 GetUsedSize() const
	{
		if (mFront <= mRear)
		{
			return mRear - mFront;
		}
		else
		{
			return BUFFER_SIZE - mFront + mRear;
		}
	}

	Int32 GetRemainSize() const
	{
		return BUFFER_SIZE - 1 - GetUsedSize();
	}

	bool Write(const Byte* inputBuffer, Int32 size)
	{
		Int32 remainSize = GetRemainSize();
		if (remainSize < size)
		{
			return false;
		}

		Int32 directWriteSize = GetDirectWriteSize();
		if (size <= directWriteSize)
		{
			memcpy(mBuffer + mRear, inputBuffer, size);
		}
		else
		{
			memcpy(mBuffer + mRear, inputBuffer, directWriteSize);
			memcpy(mBuffer, inputBuffer + directWriteSize, size - directWriteSize);
		}

		mRear += size;
		mRear %= BUFFER_SIZE;

		return true;
	}

	Int32 Peek(Byte* readBuffer, Int32 size)
	{
		const Int32 usedSize = GetUsedSize();
		Int32 peekSize;
		if (usedSize <= size)
		{
			peekSize = usedSize;
		}
		else
		{
			peekSize = size;
		}

		Int32 directReadSize = GetDIrectReadSize();
		if (peekSize <= directReadSize)
		{
			//TODO: optimize check
			memcpy(readBuffer, mBuffer + directReadSize, peekSize);
		}
		else
		{
			memcpy(readBuffer, mBuffer + mFront, directReadSize);
			memcpy(readBuffer + directReadSize, mBuffer, peekSize - directReadSize);
		}

		return peekSize;
	}

	Int32 Read(Byte* readBuffer, Int32 size)
	{
		Int32 peekSize = Peek(readBuffer, size);
		mFront += peekSize;
		mFront %= BUFFER_SIZE;

		return peekSize;
	}

	void MoveReadOffset(Int32 moveSize)
	{
		mFront += moveSize;
		mFront %= BUFFER_SIZE;
	}

	void MoveWriteOffset(Int32 moveSize)
	{
		mRear += moveSize;
		mRear %= BUFFER_SIZE;
	}

private:
	Int32 mFront;
	Int32 mRear;
	Byte mBuffer[BUFFER_SIZE];
};
}