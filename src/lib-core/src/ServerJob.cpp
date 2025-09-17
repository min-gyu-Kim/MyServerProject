#include "core/Server.hpp"
#include "ServerJob.hpp"
#include "core/Network/Endpoint.hpp"

#include <sys/epoll.h>

namespace core {

PollJob::PollJob(Server* server) : mServer(server)
{
    ASSERT(server != nullptr, "PollJob::server pointer must be not null");
}

void PollJob::Execute()
{
    epoll_event events[MAX_EVENTS];
    Int32 epollFD = mServer->GetEpollFD();
    Int32 result = epoll_wait(epollFD, events, MAX_EVENTS, -1); // TODO: add timeout
    if (result < 0) {
        fmt::println(stderr, "epoll_wait error code : {} ({})", errno, strerror(errno));
        return;
    }

    for (Int32 i = 0; i < result; ++i) {
        mServer->AddJob((IJob*)events[i].data.ptr);
    }

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
        // TEST
        mServer->OnAccepeted(clientEndpoint);
        /*
        epoll_event recvEvent{};
        recvEvent.data.fd = clientFD; // TODO: change recvJob
        recvEvent.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
        epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &recvEvent);*/
    } while (true);

    epoll_event event{};
    event.data.ptr = this;
    event.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
    epoll_ctl(epollFD, EPOLL_CTL_MOD, listenFD, &event);
}

} // namespace core