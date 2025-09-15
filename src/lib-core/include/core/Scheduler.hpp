#pragma once

namespace core {

class Acceptor;

template <typename Derived>
class SchedulerBase
{
  public:
    SchedulerBase() : mStop(false)
    {
    }

    void Run()
    {
        GetDerived().Run();
    }

    void RegistAcceptor(Acceptor* acceptor)
    {
        GetDerived().RegistAcceptor(acceptor);
    }

  protected:
    Derived& GetDerived()
    {
        return static_cast<Derived&>(*this);
    }
    const Derived& GetDerived() const
    {
        return static_cast<const Derived&>(*this);
    }

  protected:
    bool mStop;
};

} // namespace core

#ifdef __linux__
#include "Scheduler_uring.hpp"
namespace core {
using Scheduler = class SchedulerUring;
}
#else
#error "Unsupported platform"
#endif
