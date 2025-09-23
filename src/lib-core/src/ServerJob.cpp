#include "ServerJob.hpp"
#include "core/Server.hpp"
#include "core/Network/Endpoint.hpp"
#include "Network/RecvBuffer.hpp"
#include "Network/Session.hpp"
#include "Network/SessionGroup.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

namespace core {

namespace {
const Int32 sMAX_EVENTS = 126;
}

PollJob::PollJob(Server* server) : mServer(server)
{
    ASSERT(server != nullptr, "PollJob::server pointer must be not null");
}

bool PollJob::Execute()
{
    epoll_event events[sMAX_EVENTS];
    Int32 epollFD = mServer->GetEpollFD();
    do {
        Int32 result = epoll_wait(epollFD, events, sMAX_EVENTS, -1);
        if (result < 0) {
            if (EINTR == errno) {
                continue;
            }
            fmt::println(stderr, "PollJob epoll_wait error code : {} ({})", errno, strerror(errno));
        } else {
            for (Int32 i = 0; i < result; ++i) {
                mServer->AddJob((IJob*)events[i].data.ptr);
            }
        }
    } while (false);

    mServer->AddJob(this);

    return true;
}

SendPollJob::SendPollJob(Server* server) : mServer(server)
{
}

bool SendPollJob::Execute()
{
    epoll_event events[sMAX_EVENTS];
    Int32 epollFD = mServer->GetEpollFD();
    do {
        Int32 result = epoll_wait(epollFD, events, sMAX_EVENTS, -1); // TODO: add timeout
        if (result < 0) {
            if (EINTR == errno) {
                continue;
            }
            fmt::println(stderr, "SendPollJob epoll_wait error code : {} ({})", errno,
                         strerror(errno));
        } else {
            for (Int32 i = 0; i < result; ++i) {
                if (events[i].data.ptr != nullptr) {
                    mServer->AddJob((IJob*)events[i].data.ptr);
                }
            }
        }
    } while (false);

    mServer->AddJob(this);

    return true;
}

AcceptJob::AcceptJob(Server* server, SessionGroup* sessionGroup)
    : mServer(server), mSessionGroup(sessionGroup)
{
    ASSERT(server != nullptr, "AcceptJob::server pointer must be not null");
    ASSERT(sessionGroup != nullptr, "AcceptJob::sessionGroup pointer must be not null");
}

bool AcceptJob::Execute()
{
    SocketFD listenFD = mServer->GetListenSocket();
    Int32 epollFD = mServer->GetEpollFD();
    sockaddr_storage clientAddress{};
    socklen_t addressLen = sizeof(clientAddress);

    do {
        SocketFD clientFD = accept(listenFD, (sockaddr*)&clientAddress, &addressLen);
        if (clientFD == sINVALID_SOCKET_FD) {
            if (EAGAIN == errno) {
                break;
            }
            if (EINTR == errno) {
                continue;
            }

            fmt::println(stderr, "accept error code : {} ({})", errno, strerror(errno));
            break;
        }

        Endpoint clientEndpoint((const sockaddr*)&clientAddress);
        if (!mServer->OnAccepeted(clientEndpoint)) {
            close(clientFD);
            continue;
        }

        SessionID clientSession = mSessionGroup->AddSession(clientFD);
        Session* session = mSessionGroup->GetSession(clientSession);
        Int32 flags = fcntl(clientFD, F_GETFL, 0);
        fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);

        int delayZeroOpt = 1;
        setsockopt(clientFD, SOL_SOCKET, TCP_NODELAY, (const char*)&delayZeroOpt,
                   sizeof(delayZeroOpt));

        epoll_event sendEvent{};
        sendEvent.data.ptr = nullptr;
        sendEvent.events = EPOLLOUT | EPOLLONESHOT;
        epoll_ctl(mServer->GetSendPollFD(), EPOLL_CTL_ADD, clientFD, &sendEvent);

        epoll_event recvEvent{};
        recvEvent.data.ptr = session->GetRecvJob();
        recvEvent.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
        epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &recvEvent);

        mServer->OnConnected(clientSession);
    } while (true);

    return true;
}

void AcceptJob::Complete()
{
    Int32 epollFD = mServer->GetEpollFD();
    SocketFD listenFD = mServer->GetListenSocket();

    epoll_event event{};
    event.data.ptr = this;
    event.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
    epoll_ctl(epollFD, EPOLL_CTL_MOD, listenFD, &event);
}

RecvJob::RecvJob(Session* session, Server* server) : mSession(session), mServer(server)
{
}

bool RecvJob::Execute()
{
    do {
        if (mSession->GetSocketFD() == sINVALID_SOCKET_FD) {
            mSession->SetReceiveState(false);
            mSession->Disconnect();
            return false;
        }

        ssize_t result = recv(mSession->GetSocketFD(), mRecvBuffer.GetRearBufferPtr(),
                              mRecvBuffer.GetDirectWriteSize(), 0);
        if (result == 0) {
            mSession->SetReceiveState(false);
            mSession->Disconnect();
            return false;
        } else if (result < 0) {
            if (EAGAIN == errno) {
                break;
            } else if (EINTR == errno) {
                continue;
            } else if (EPIPE == errno || ECONNRESET == errno) {
                mSession->SetReceiveState(false);
                mSession->Disconnect();
                return false;
            }

            fmt::println(stderr, "recv error: {} ({})", errno, strerror(errno));
            break;
        }

        mRecvBuffer.MoveWriteOffset(result);
    } while (true);

    return true;
}

void RecvJob::Complete()
{
    const Int32 usedSize = mRecvBuffer.GetUsedSize();

    if (usedSize > 0) {
        const Int32 readSize =
            mServer->OnRecv(mSession->GetSessionID(), mRecvBuffer.GetFrontBufferPtr(),
                            mRecvBuffer.GetDIrectReadSize());

        if (readSize > 0) {
            mRecvBuffer.MoveReadOffset(readSize);

            if (readSize == usedSize) {
                mRecvBuffer.Reset();
            }
        }
    }

    epoll_event event{};
    event.data.ptr = this;
    event.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
    if (0 != epoll_ctl(mServer->GetEpollFD(), EPOLL_CTL_MOD, mSession->GetSocketFD(), &event)) {
        bool isValidSocket = (mSession->GetSocketFD() != sINVALID_SOCKET_FD);

        if (isValidSocket) {
            fmt::println(stderr, "epoll_ctl error : {} ({})", errno, strerror(errno));
            return;
        } else {
            mSession->SetReceiveState(false);
            mSession->Disconnect();
        }
    }
}

SendJob::SendJob(class Session* session, Server* server) : mSession(session), mServer(server)
{
}

bool SendJob::Execute()
{
    mSession->DoSend();
    fmt::println("SendJob complete");

    return false;
}

void SendJob::Reset()
{
    mSendPackets.clear();
}

void SendJob::AddPackets(void* buffer, size_t size)
{
    mSendPackets.emplace_back(iovec{buffer, size});
}

} // namespace core