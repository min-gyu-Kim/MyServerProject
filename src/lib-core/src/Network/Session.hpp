#pragma once

#include "core/Types.hpp"
#include "core/Lock.hpp"
#include "RecvBuffer.hpp"
#include "../ServerJob.hpp"

namespace core {

class Server;

class Session
{
  public:
    Session() : mRecvJob(this, nullptr), mSendJob(this, nullptr)
    {
    }
    Session(Server* server, SessionID sessionID, SocketFD sessionFD)
        : mServer(server), mSessionID(sessionID), mSessionFD(sessionFD), mIsRecving(true),
          mIsSending(false), mRecvJob(this, server), mSendJob(this, server)
    {
    }

    ~Session()
    {
    }

    SessionID GetSessionID() const
    {
        return mSessionID;
    }

    SocketFD GetSocketFD() const
    {
        return mSessionFD;
    }

    RecvJob* GetRecvJob()
    {
        return &mRecvJob;
    }

    SendJob* GetSendJob()
    {
        return &mSendJob;
    }

    void SetReceiveState(bool state)
    {
        mIsRecving = state;
    }

    void SetSendState(bool state)
    {
        mIsSending = state;
    }

    void Disconnect();
    void DoSend();
    void Send(Byte* buffer, size_t bufferSize);

  private:
    SessionID mSessionID;
    SocketFD mSessionFD;

    bool mIsRecving;
    RecvJob mRecvJob;

    bool mIsSending;
    SendJob mSendJob;
    Mutex mPacketMutex;
    Vector<iovec> mPackets;

    Server* mServer;
};
} // namespace core