#pragma once

#include "Types.hpp"
#include "Debug.hpp"
#include <asm-generic/errno.h>

#if defined(__GNUC__) || defined(__clang__)
#include <pthread.h>
#elif defined(MSVC)
#else
#error "Not supported"
#endif

namespace core{

template <typename LockObject>
class LockGuard final
{
public:
	LockGuard(const LockObject& obj) : mObj(obj)
	{
		mObj->Lock();
	}

	~LockGuard()
	{
		mObj->Unlock();
	}
private:
	LockObject* mObj;
};

class NullLock
{
public:
	NullLock() = default;
	~NullLock() = default;

	void Lock() {}
	void Unlock() {}
	bool TryLock() const { return true; }
};

#if defined(__GNUC__) || defined(__clang__)
class Mutex
{
public:
	Mutex()
	{
#if defined(DEBUG)
		pthread_mutexattr_t attribute = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#else
		pthread_mutexattr_t attribute = PTHREAD_MUTEX_INITIALIZER;
#endif
		pthread_mutex_init(&mMutex, &attribute);
	}

	~Mutex()
	{
		pthread_mutex_destroy(&mMutex);
	}

	void Lock()
	{
		const Int32 result = pthread_mutex_lock(&mMutex);
#if defined(DEBUG)
		ASSERT(result == 0, "Lock() error: {} ({})", result, strerror(result));
#endif
	}

	void Unlock()
	{
		pthread_mutex_unlock(&mMutex);
	}

	bool TryLock()
	{
		const Int32 result = pthread_mutex_trylock(&mMutex);
		const bool isLock = (result == 0);

#if defined(DEBUG)
		ASSERT(result != EDEADLOCK, "TryLock() detect deadlock");
#endif

		return isLock;
	}

private:
	pthread_mutex_t mMutex;
};
#else
#endif

}