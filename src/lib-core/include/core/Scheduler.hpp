#pragma once

#include "Container.hpp"
#include "Types.hpp"
#include "Network/Endpoint.hpp"

namespace core {

class Acceptor;
class IAcceptorEvent;
class ISessionEvent;

typedef struct TagAcceptorCreateParameter
{
  Endpoint mEndpoint;
  Int32 mMaxSessionCount;
  IAcceptorEvent* mAcceptHandler;
  ISessionEvent* mSessionHandler;
} AcceptorCreateParameter;

template <typename Derived>
class SchedulerBase
{
  protected:
    SchedulerBase() : mStop(false)
    {
    }

  public:
    void Run()
    {
        GetDerived().Run();
    }

    AcceptorID CreateAcceptor(const AcceptorCreateParameter& createParameter);

    void CancelAcceptor(const AcceptorID& acceptorID);

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
