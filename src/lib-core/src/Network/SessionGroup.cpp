#include "SessionGroup.hpp"

namespace core {

namespace {
const Int64 sSessionUnqueMask = 0xFFFFFFFFFFFF0000;
const Int64 sSessionIndexMask = 0x000000000000FFFF;
Int64 sUniqueNumber = 0;

Int32 GetIndex(SessionID sessionID)
{
    return sessionID & sSessionIndexMask;
}

Int64 GetUniqueNumber(SessionID sessionID)
{
    Int64 uniqueNumber = (sessionID & sSessionUnqueMask) >> 48;
    return uniqueNumber;
}
} // namespace

SessionGroup::SessionGroup(Server* server, Int32 maxSessions)
    : mServer(server), mSessionVector(maxSessions), mMaxSession(maxSessions)
{
    ASSERT(server != nullptr, "SessionGroup construct parameter error, server pointer is null");
    ASSERT(mMaxSession > 0 && mMaxSession < 65535,
           fmt::format("max session range (0, 65535), maxSessions value : {}", maxSessions));

    Int32 index = maxSessions;
    for (; index >= 0; --index) {
        mSessionIndexPool.push(index);
    }
}

SessionGroup::~SessionGroup()
{
}

SessionID SessionGroup::AddSession(SocketFD clientFD)
{
    ASSERT(clientFD != sINVALID_SOCKET_FD, "Add Session, clientFD is invalid value");

    Int32 index = AllocSession();

    Int64 uniqueNumber = __sync_fetch_and_add(&sUniqueNumber, 1);
    uniqueNumber <<= 48;
    SessionID sessionID = uniqueNumber | index;

    Session* session = &mSessionVector[index];
    new (session) Session(mServer, sessionID, clientFD);

    return sessionID;
}

bool SessionGroup::DeleteSession(SessionID sessionID)
{
    Int32 index = GetIndex(sessionID);
    Session* session = &mSessionVector[index];
    if (session->GetSessionID() != sessionID) {
        return false;
    }

    FreeSession(index);
    return true;
}

Session* SessionGroup::GetSession(SessionID sessionID)
{
    Int32 index = GetIndex(sessionID);
    Session* session = &mSessionVector[index];
    if (session->GetSessionID() != sessionID) {
        return nullptr;
    }

    return session;
}

Int32 SessionGroup::AllocSession()
{
    LockGuard<Mutex> lock(mStackMutex);

    if (mSessionIndexPool.empty()) {
        return sINVALID_ID;
    }

    Int32 index = mSessionIndexPool.top();
    mSessionIndexPool.pop();
    return index;
}

void SessionGroup::FreeSession(Int32 index)
{
    LockGuard<Mutex> lock(mStackMutex);
    mSessionIndexPool.push(index);
}

} // namespace core