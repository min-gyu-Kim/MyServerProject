#include "core/Server.hpp"
#include "WorkerThreadPool.hpp"
#include "ServerJob.hpp"
#include "Network/SessionGroup.hpp"

#include <sys/epoll.h>

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

Server::Server()
    : mListenFD(sINVALID_SOCKET_FD), mIsRunning(false), mWorkerThreadPool(nullptr),
      mSessionGroup(nullptr)
{
}

Server::~Server()
{
    Stop();

    if (mPollJob != nullptr) {
        delete mPollJob;
        mPollJob = nullptr;
    }

    if (mAcceptJob != nullptr) {
        delete mAcceptJob;
        mAcceptJob = nullptr;
    }

    if (mWorkerThreadPool != nullptr) {
        delete mWorkerThreadPool;
        mWorkerThreadPool = nullptr;
    }

    if (mSessionGroup != nullptr) {
        delete mSessionGroup;
        mSessionGroup = nullptr;
    }
}

bool Server::Start(const Endpoint& serverEndpoint, size_t workerThreadCount)
{
    mEpollFD = epoll_create1(0);
    if (mEpollFD == sINVALID_ID) {
        fmt::println(stderr, "epoll_create error : {} ({})", errno, strerror(errno));
        return false;
    }

    mSendPollFD = epoll_create1(0);
    if (mSendPollFD == sINVALID_ID) {
        fmt::println(stderr, "epoll_create error : {} ({})", errno, strerror(errno));
        return false;
    }

    SessionGroup* sessionGroup = new SessionGroup(this, 5000);
    mSessionGroup = sessionGroup;

    WorkerThreadPool* workerThreadPool = new WorkerThreadPool();
    workerThreadPool->Start(workerThreadCount);
    mWorkerThreadPool = workerThreadPool;

    mListenFD = CreateListenSocket(serverEndpoint);

    mIsRunning = true;

    PollJob* pollJob = new PollJob(this);
    mPollJob = pollJob;
    AddJob(pollJob);

    AcceptJob* acceptJob = new AcceptJob(this, sessionGroup);
    mAcceptJob = acceptJob;

    epoll_event event{};
    event.data.ptr = acceptJob;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
    epoll_ctl(mEpollFD, EPOLL_CTL_ADD, mListenFD, &event);

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

    if (mEpollFD != sINVALID_ID) {
        close(mEpollFD);
        mEpollFD = sINVALID_ID;
    }

    if (mSendPollFD != sINVALID_ID) {
        close(mSendPollFD);
        mSendPollFD = sINVALID_ID;
    }

    WorkerThreadPool* threadPool = (WorkerThreadPool*)mWorkerThreadPool;
    threadPool->Stop();
}

void Server::Send(SessionID sessionID, const Byte* buffer, size_t bufferSize)
{
    SessionGroup* sessionGroup = (SessionGroup*)mSessionGroup;
    Session* session = sessionGroup->GetSession(sessionID);
    if (session == nullptr) {
        return;
    }

    session->Send((Byte*)buffer, bufferSize);
}

void Server::AddJob(IJob* job)
{
    WorkerThreadPool* threadPool = (WorkerThreadPool*)mWorkerThreadPool;
    threadPool->AddJob(job);
}

SessionID Server::AddSession(SocketFD clientFD)
{
    SessionGroup* sessionGroup = (SessionGroup*)mSessionGroup;
    return sessionGroup->AddSession(clientFD);
}

bool Server::DeleteSession(SessionID sessionID)
{
    SessionGroup* sessionGroup = (SessionGroup*)mSessionGroup;
    return sessionGroup->DeleteSession(sessionID);
}

void Server::CloseListenSocket()
{
    if (mListenFD != sINVALID_SOCKET_FD) {
        close(mListenFD);
        mListenFD = sINVALID_SOCKET_FD;
    }
}

} // namespace core