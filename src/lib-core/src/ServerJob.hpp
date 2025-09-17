#pragma once

#include "core/Job.hpp"
#include "core/Types.hpp"

namespace core {
class Server;

class PollJob : public IJob
{
  private:
    enum { MAX_EVENTS = 126 };

  public:
    PollJob() = delete;
    PollJob(Server* server);
    void Execute() override;

  private:
    Server* mServer;
};

class AcceptJob : public IJob
{
  public:
    AcceptJob() = delete;
    AcceptJob(Server* server);
    void Execute() override;

  private:
    Server* mServer;
};
/*
class RecvJob : public IJob
{
  public:
    RecvJob() = delete;
    RecvJob(class Session* session, Server* server);

    void Execute() override;

  private:
    class Session* mSession;
    Server* mServer;
};
*/
} // namespace core