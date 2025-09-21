#pragma once

#include "core/Container.hpp"
#include "core/Lock.hpp"
#include "Session.hpp"

namespace core {
class Server;
class SessionGroup
{
  public:
    SessionGroup() = delete;
    SessionGroup(Server* server, Int32 maxSessions);
    ~SessionGroup();

    SessionID AddSession(SocketFD clientFD);
    bool DeleteSession(SessionID sessionID);

    Session* GetSession(SessionID sessionID);

  private:
    Int32 AllocSession();
    void FreeSession(Int32 index);

  private:
    Server* mServer;
    Int32 mMaxSession;
    Vector<Session> mSessionVector;

    // array index pool
    Mutex mStackMutex;
    Stack<Int32> mSessionIndexPool;
};
} // namespace core