#include "Session.hpp"
#include "core/Server.hpp"

#include <sys/epoll.h>
#include <sys/uio.h>

namespace core {

void Session::Disconnect()
{
    SocketFD oldFD = __sync_lock_test_and_set(&mSessionFD, sINVALID_SOCKET_FD);

    if (oldFD == sINVALID_SOCKET_FD) {
        if (!mIsRecving && !mIsSending) {
            mServer->OnDisconnected(mSessionID);
            // mServer->DeleteSession(mSessionID);
        }
        return;
    }

    epoll_ctl(mServer->GetEpollFD(), EPOLL_CTL_DEL, oldFD, nullptr);
    close(oldFD);

    if (!mIsRecving && !mIsSending) {
        mServer->OnDisconnected(mSessionID);
        // mServer->DeleteSession(mSessionID);
    }
}

void Session::Send(Byte* buffer, size_t bufferSize)
{
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }

    if (__sync_lock_test_and_set(&mIsSending, true)) {
        LockGuard<Mutex> lock(mPacketMutex);
        mPackets.emplace_back(iovec{buffer, bufferSize});
        return;
    }

    epoll_event event{};
    event.data.ptr = &mSendJob;
    event.events = EPOLLOUT | EPOLLONESHOT;
    epoll_ctl(mServer->GetSendPollFD(), EPOLL_CTL_MOD, mSessionFD, &event);
}

} // namespace core