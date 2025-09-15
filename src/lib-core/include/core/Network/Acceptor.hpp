#pragma once

#include <netinet/in.h>
#include "Endpoint.hpp"
#include "../Types.hpp"
#include "../Scheduler.hpp"

namespace core {
class Acceptor
{
  public:
    Acceptor(Scheduler* scheduler) : mListenFd(-1), mScheduler(scheduler) {};
    virtual ~Acceptor();

    bool Initialize(const Endpoint& endpoint);

    virtual bool OnAccept(const sockaddr* clientAddr) = 0;

    SocketFD GetListenFD() const
    {
        return mListenFd;
    }

  private:
    SocketFD mListenFd;
    Scheduler* mScheduler;
};
} // namespace core