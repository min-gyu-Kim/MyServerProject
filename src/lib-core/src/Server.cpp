#include "core/Server.hpp"

namespace core {

namespace {

SocketFD CreateListenSocket(const Endpoint& serverEndpoint)
{
    SocketFD listenFD = socket(serverEndpoint.GetIPVersion() == Endpoint::IPv4 ? AF_INET : AF_INET6,
                               SOCK_STREAM, 0);
    if (listenFD == sINVALID_SOCKET_FD) {
        ASSERT(false, fmt::format("socket() failed. errno: {}", errno));
        return sINVALID_SOCKET_FD;
    }

    int opt = 1;
    if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        ASSERT(false, fmt::format("setsockopt() failed. errno: {}", errno));
        close(listenFD);
        return sINVALID_SOCKET_FD;
    }

    Int32 flags = fcntl(listenFD, F_GETFL, 0);
    fcntl(listenFD, F_SETFL, flags | O_NONBLOCK);

    if (bind(listenFD, serverEndpoint.GetAddress(),
             (serverEndpoint.GetIPVersion() == Endpoint::IPv4) ? sizeof(sockaddr_in)
                                                               : sizeof(sockaddr_in6)) < 0) {
        ASSERT(false, fmt::format("bind() failed. errno: {}", errno));
        close(listenFD);
        return sINVALID_SOCKET_FD;
    }

    if (listen(listenFD, SOMAXCONN) < 0) {
        ASSERT(false, fmt::format("listen() failed. errno: {}", errno));
        close(listenFD);
        return sINVALID_SOCKET_FD;
    }

    return listenFD;
}
} // namespace

Server::Server() : mListenFD(sINVALID_SOCKET_FD), mIsRunning(false)
{
}

Server::~Server()
{
    Stop();
}

bool Server::Start(const Endpoint& serverEndpoint, size_t workerThreadCount)
{
    mIsRunning = true;
    mWorkerThreads.resize(workerThreadCount);

    return true;
}

void Server::Stop()
{
    if (!mIsRunning) {
        return;
    }

    CloseListenSocket();

    mIsRunning = false;

    if (mListenFD != sINVALID_SOCKET_FD) {
        close(mListenFD);
        mListenFD = sINVALID_SOCKET_FD;
    }

    for (pthread_t& thread : mWorkerThreads) {
        pthread_join(thread, nullptr);
    }
    mWorkerThreads.clear();
}

void Server::CloseListenSocket()
{
    if (mListenFD != sINVALID_SOCKET_FD) {
        close(mListenFD);
        mListenFD = sINVALID_SOCKET_FD;
    }
}

} // namespace core