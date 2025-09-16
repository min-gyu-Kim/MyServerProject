#pragma once

#include <pthread.h>
#include "Container.hpp"
#include "Types.hpp"
#include "Lock.hpp"
#include "Network/Endpoint.hpp"

namespace core {
class Server
{
  public:
    Server();
    ~Server();

    bool Start(const Endpoint& serverEndpoint, size_t workerThreadCount);
    void Stop();

    virtual bool OnAccepeted(const Endpoint& clientEndpoint) = 0;
    virtual bool OnRecv(SessionID sessionID, const class Packet* packet) = 0;
    virtual void OnDisconnected(SessionID sessionID) = 0;

  private:
    void PollEvents();
    void CloseListenSocket();

  private:
    SocketFD mListenFD;
    Int32 mEpollFD;
    bool mIsRunning;
    Vector<pthread_t> mWorkerThreads;
};

} // namespace core