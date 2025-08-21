#pragma once

#include "../Container.hpp"

#if defined(_MSVC)
#else

#include <pthread.h>

namespace core {
inline void *CreateThread(void *(*start_routine)(void *), void *arg)
{
    pthread_t thread = -1;
    pthread_create(&thread, nullptr, start_routine, arg);
    return reinterpret_cast<void *>(thread); // NOLINT
}
} // namespace core
#endif

namespace core {
class Thread
{
  public:
    explicit Thread() : mThread(nullptr) {}

    Thread(void *(*start_routine)(void *), void *arg)
    {
        mThread = CreateThread(start_routine, arg);
    }

    ~Thread()
    {
        if (mThread != nullptr) {
            pthread_join(reinterpret_cast<pthread_t>(mThread), nullptr);
        }
    }

  private:
    void *mThread;
};
} // namespace core