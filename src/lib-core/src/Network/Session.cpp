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
    if (buffer == nullptr || bufferSize <= 0) {
        return;
    }

    {
        LockGuard<Mutex> lock(mPacketMutex);
        mPackets.emplace_back(iovec{buffer, bufferSize});
    }

    if (__sync_lock_test_and_set(&mIsSending, true)) {
        return;
    }

    DoSend();

    epoll_event event{};
    event.data.ptr = &mSendJob;
    event.events = EPOLLOUT | EPOLLONESHOT;
    epoll_ctl(mServer->GetSendPollFD(), EPOLL_CTL_MOD, mSessionFD, &event);
}

void Session::DoSend()
{
    Vector<iovec> sendPackets;
    {
        LockGuard<Mutex> lock(mPacketMutex);
        sendPackets.swap(mPackets);
    }

    if (sendPackets.empty()) {
        mIsSending = false;
        return;
    }

    Int32 totalSendBytes = 0;
    for (Int32 i = 0; i < sendPackets.size(); ++i) {
        totalSendBytes += sendPackets[i].iov_len;
    }

    ASSERT(totalSendBytes > 0, "send buffers size == 0");

    Int32 result = 0;
    do {
        if (mSessionFD == sINVALID_SOCKET_FD) {
            mIsSending = false;
            Disconnect();
            return;
        }

        result = writev(mSessionFD, sendPackets.data(), sendPackets.size());
        if (result < 0) {
            if (EAGAIN == errno) {
                mIsSending = false; // receiver buffer full
                Disconnect();
                return;
            }
            if (EINTR == errno) {
                continue;
            } else {
                mIsSending = false;
                Disconnect();
            }

            // TODO: log
        }

        if (result == totalSendBytes) {
            mIsSending = false;
            return;
        } else {
            break;
        }

    } while (true);

    Int32 startOffset = 0;
    Int32 index = 0;
    for (; index < sendPackets.size(); ++index) {
        if (result < sendPackets[index].iov_len) {
            startOffset = sendPackets[index].iov_len - result;
            break;
        }
    }

    mSendJob.Reset();
    for (; index < sendPackets.size(); ++index) {
        Byte* sendBufferPtr = (Byte*)sendPackets[index].iov_base + startOffset;
        size_t sendSize = sendPackets[index].iov_len - startOffset;
        mSendJob.AddPackets(sendBufferPtr, sendSize);

        startOffset = 0;
    }

    epoll_event event{};
    event.data.ptr = &mSendJob;
    event.events = EPOLLOUT | EPOLLONESHOT;
    epoll_ctl(mServer->GetSendPollFD(), EPOLL_CTL_MOD, mSessionFD, &event);
}

} // namespace core