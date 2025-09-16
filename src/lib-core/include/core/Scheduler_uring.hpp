#pragma once

#include <liburing.h>
#include <pthread.h>
#include "Scheduler.hpp"

namespace core {

class SchedulerUring
{
  public:
    SchedulerUring();
    ~SchedulerUring();

    void Run();

    AcceptorID CreateAcceptor(const AcceptorCreateParameter& param);

  private:
    bool mStop;
    pthread_mutex_t mMutex;
    io_uring mRing;
};
} // namespace core