#pragma once

#include <liburing.h>
#include <pthread.h>
#include "Scheduler.hpp"

namespace core {

class SchedulerUring : public SchedulerBase<SchedulerUring>
{
  public:
    SchedulerUring();
    ~SchedulerUring();

    void Run();

    void RegistAcceptor(Acceptor* acceptor);

  private:
    pthread_mutex_t mMutex;
    io_uring mRing;
};
} // namespace core