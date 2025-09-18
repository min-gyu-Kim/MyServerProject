#pragma once

#include <pthread.h>
#include "Container.hpp"
#include "Types.hpp"
#include "Lock.hpp"
#include "Network/Endpoint.hpp"

namespace core {

class IJob;
class Server
{
  public:
    Server();
    ~Server();

    bool Start(const Endpoint& serverEndpoint, size_t workerThreadCount);
    void Stop();

    void AddJob(IJob* job);

    virtual bool OnAccepeted(const Endpoint& clientEndpoint) = 0;
    virtual Int32 OnRecv(SessionID sessionID, const Byte* buffer, Int32 inputSize) = 0;
    virtual void OnDisconnected(SessionID sessionID) = 0;

  public:
    SocketFD GetListenSocket() const
    {
        return mListenFD;
    }

    Int32 GetEpollFD() const
    {
        return mEpollFD;
    }

  private:
    void PollEvents();
    void CloseListenSocket();

  private:
    SocketFD mListenFD;
    Int32 mEpollFD;
    bool mIsRunning;
    void* mWorkerThreadPool;
};

} // namespace core