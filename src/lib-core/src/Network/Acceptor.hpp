#pragma once

#include <netinet/in.h>
#include "core/Network/Endpoint.hpp"
#include "core/Network/EventHandlerInterface.hpp"
#include "core/Types.hpp"
#include "core/Scheduler.hpp"

namespace core {
class Acceptor
{
  public:
    Acceptor(SchedulerUring* scheduler) : 
      mListenFd(sINVALID_SOCKET_FD), 
      mScheduler(scheduler)
    {};
    ~Acceptor();

    bool Initialize(const AcceptorCreateParameter& param);
    void OnEvent(SocketFD clientFD, const sockaddr* clientAddress);

    SocketFD GetListenFD() const
    {
        return mListenFd;
    }

  private:
    SocketFD mListenFd;
    SchedulerUring* mScheduler;
    IAcceptorEvent* mEventHandler;
};
} // namespace core