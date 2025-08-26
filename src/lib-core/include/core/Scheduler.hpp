#pragma once

#include "Types.hpp"

namespace core {

template <typename Derived>
class SchedulerBase
{
  public:
    SchedulerBase() : mStop(false) {}

    void Run() { GetDerived().Run(); }

  protected:
    Derived &GetDerived() { return static_cast<Derived &>(*this); }
    const Derived &GetDerived() const
    {
        return static_cast<const Derived &>(*this);
    }

  protected:
    bool mStop;
};

} // namespace core
