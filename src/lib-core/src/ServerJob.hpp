#pragma once

#include "core/Job.hpp"
#include "core/Types.hpp"
#include "core/Container.hpp"
#include "Network/RecvBuffer.hpp"

#include <sys/uio.h>

namespace core {
class Server;
class SessionGroup;

class PollJob : public IJob
{
  public:
    PollJob() = delete;
    PollJob(Server* server);
    bool Execute() override;

  private:
    Server* mServer;
};

class SendPollJob : public IJob
{
  private:
    SendPollJob() = delete;
    SendPollJob(Server* server);

    bool Execute() override;

  private:
    Server* mServer;
};

class AcceptJob : public IJob
{
  public:
    AcceptJob() = delete;
    AcceptJob(Server* server, SessionGroup* sessionGroup);
    bool Execute() override;
    void Complete() override;

  private:
    Server* mServer;
    SessionGroup* mSessionGroup;
};

class RecvJob : public IJob
{
  public:
    RecvJob() = delete;
    RecvJob(class Session* session, Server* server);

    bool Execute() override;
    void Complete() override;

  private:
    class Session* mSession;
    Server* mServer;
    RecvBuffer mRecvBuffer;
};

class SendJob : public IJob
{
  public:
    SendJob() = delete;
    SendJob(class Session* session, Server* server);

    bool Execute() override;

    void Reset();
    void AddPackets(void* buffer, size_t size);

  private:
    class Session* mSession;
    Server* mServer;
    Vector<iovec> mSendPackets;
};

} // namespace core