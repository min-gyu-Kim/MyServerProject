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

    void Send(SessionID sessionID, const Byte* buffer, size_t bufferSize);

    void AddJob(IJob* job);
    SessionID AddSession(SocketFD clientFD);
    bool DeleteSession(SessionID sessionID);

    virtual bool OnAccepeted(const Endpoint& clientEndpoint) = 0;
    virtual void OnConnected(const SessionID sessionID) = 0;
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

    Int32 GetSendPollFD() const
    {
        return mSendPollFD;
    }

  private:
    void PollEvents();
    void CloseListenSocket();

  private:
    SocketFD mListenFD;
    Int32 mEpollFD;
    Int32 mSendPollFD;
    bool mIsRunning;
    void* mWorkerThreadPool;
    void* mSessionGroup;
    void* mPollJob;
    void* mAcceptJob;
};

} // namespace core