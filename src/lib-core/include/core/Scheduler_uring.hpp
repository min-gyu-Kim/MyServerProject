#pragma once

#include <liburing.h>
#include "Scheduler.hpp"

namespace core {

class SchedulerUring : public SchedulerBase<SchedulerUring>
{
  public:
    SchedulerUring();
    ~SchedulerUring();

    void Run();

  private:
};
} // namespace core