#include "ServerJob.hpp"
#include "core/Server.hpp"
#include "core/Network/Endpoint.hpp"
#include "Network/RecvBuffer.hpp"
#include "Network/Session.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

namespace core {

PollJob::PollJob(Server* server) : mServer(server)
{
    ASSERT(server != nullptr, "PollJob::server pointer must be not null");
}

void PollJob::Execute()
{
    epoll_event events[MAX_EVENTS];
    Int32 epollFD = mServer->GetEpollFD();
    do{
        Int32 result = epoll_wait(epollFD, events, MAX_EVENTS, -1); // TODO: add timeout
        if (result < 0) {
            if(EINTR == errno)
            {
                continue;
            }
            fmt::println(stderr, "epoll_wait error code : {} ({})", errno, strerror(errno));
        }
        else
        {
            for (Int32 i = 0; i < result; ++i) {
                mServer->AddJob((IJob*)events[i].data.ptr);
            }
        }
    } while(false);

    mServer->AddJob(this);
}

AcceptJob::AcceptJob(Server* server) : mServer(server)
{
    ASSERT(server != nullptr, "AcceptJob::server pointer must be not null");
}

void AcceptJob::Execute()
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
        // SessionID clientSession = mServer->AddSession(clientFD, clientAddress);
        Session* session = new Session(mServer, 0, clientFD);
        Int32 flags =fcntl(clientFD, F_GETFL, 0);
        fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);

        int delayZeroOpt = 1;
        setsockopt(clientFD, SOL_SOCKET, TCP_NODELAY, (const char*)&delayZeroOpt, sizeof(delayZeroOpt));

        // TEST
        mServer->OnAccepeted(clientEndpoint);
        
        RecvJob* recvJob = new RecvJob(session, mServer);

        epoll_event recvEvent{};
        recvEvent.data.ptr = recvJob; // TODO: change recvJob
        recvEvent.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
        epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &recvEvent);
    } while (true);

    epoll_event event{};
    event.data.ptr = this;
    event.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
    epoll_ctl(epollFD, EPOLL_CTL_MOD, listenFD, &event);
}

RecvJob::RecvJob(Session* session, Server* server) :
    mSession(session), mServer(server)
{
}

void RecvJob::Execute()
{
    RecvBuffer* buffer = mSession->GetRecvBuffer();
    do {
        Int32 result = recv(mSession->GetSocketFD(), buffer->GetRearBufferPtr(), buffer->GetDirectWriteSize(), 0);
        if(result == 0)
        {
            //close()
            return;
        } else if(result < 0)
        {
            if(EAGAIN == errno)
            {
                break;
            }
            else if(EINTR == errno)
            {
                break;
            }

            fmt::println(stderr, "recv error: {} ({})", errno, strerror(errno));
            break;
        }

        buffer->MoveWriteOffset(result);
    } while(true);

    if(buffer->GetUsedSize())
    {
        Int32 readSize = mServer->OnRecv(mSession->GetSessionID(), buffer->GetFrontBufferPtr(), buffer->GetDIrectReadSize());
        if(readSize > 0)
        {
            buffer->MoveReadOffset(readSize);
        }
    }

    epoll_event event{};
    event.data.ptr = this;
    event.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
    epoll_ctl(mServer->GetEpollFD(), EPOLL_CTL_MOD, mSession->GetSocketFD(), &event);
}

} // namespace core