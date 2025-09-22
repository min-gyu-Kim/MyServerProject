#include "PollThread.hpp"
#include "Session.hpp"
#include <cassert>
#include <process.h>
#include <WS2tcpip.h>

namespace test {

namespace {

void HandleError(int lineNumber)
{
    int errorCode = WSAGetLastError();
    char errorMessage[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage, sizeof(errorMessage),
                   nullptr);
    fprintf(stderr, "PollThread.cpp(:%d) error code : %d (%s)\n", lineNumber, errorCode,
            errorMessage);
}

} // namespace

extern thread_local LONGLONG sMAX_RTT;
extern thread_local LONGLONG sRECV_COUNT;
extern thread_local LONGLONG sSEND_COUNT;

PollThread::PollThread(int maxClientCount)
    : mMaxClientCount(maxClientCount), mCurrentClientCount(0), mIsRunning(true)
{
    assert(maxClientCount > 0);
}

PollThread::~PollThread()
{
    WaitForSingleObject(mThreadHandle, INFINITE);
}

void PollThread::Create()
{
    uint32_t threadID;
    mThreadHandle =
        (HANDLE)_beginthreadex(nullptr, 0, &PollThread::PollThreadFunction, this, 0, &threadID);

    fprintf(stdout, "Create PollThread ID: %u\n", threadID);
}

void PollThread::Loop()
{
    std::vector<WSAPOLLFD> pollFDs;
    for (const auto& session : mPollFDs) {
        WSAPOLLFD pollFD{};
        pollFD.fd = session.first;
        pollFD.events = POLLIN;
        if (session.second->mSendBuffer.GetUsedSize() > 0) {
            pollFD.events |= POLLOUT;
        }

        pollFDs.push_back(pollFD);
    }

    int result = WSAPoll(pollFDs.data(), pollFDs.size(), INFINITE);
    assert(result != 0);
    if (result == SOCKET_ERROR) {
        HandleError(__LINE__);
        return;
    }

    for (const auto& client : pollFDs) {
        bool recvResult = true;
        bool sendResult = true;
        auto iter = mPollFDs.find(client.fd);
        assert(iter != mPollFDs.end());

        if (client.revents & POLLRDNORM) {
            recvResult = iter->second->DoRecv();
            sRECV_COUNT++;
        }

        if (client.revents & POLLWRNORM) {
            sendResult = iter->second->DoSend();
            sSEND_COUNT++;
        }

        if (client.revents & POLLERR) {
            int err = 0;
            int len = sizeof(err);
            getsockopt(client.fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            fprintf(stderr, "%lld] POLLERR error code: %d\n", client.fd, err);

            closesocket(client.fd);
            delete iter->second;
            mPollFDs.erase(iter);
            continue;
        }

        if (!sendResult || !recvResult) {
            delete iter->second;
            mPollFDs.erase(iter);
        }
    }

    fprintf(stdout, "sMAX_RTT: %lld, sRECV_COUNT:%lld, sSEND_COUNT:%lld\n", sMAX_RTT, sRECV_COUNT,
            sSEND_COUNT);
}

void PollThread::Clear()
{
    for (auto& pollFd : mPollFDs) {
        if (pollFd.first == INVALID_SOCKET) {
            continue;
        }

        delete pollFd.second;

        closesocket(pollFd.first);
    }

    mPollFDs.clear();
}

void PollThread::Stop()
{
    mIsRunning = false;
}

unsigned int __stdcall PollThread::PollThreadFunction(void* arg)
{
    assert(arg != nullptr);
    PollThread* thisPtr = (PollThread*)arg;
    thisPtr->Run();
    return 0;
}

void PollThread::Run()
{
    ConnectClients();
    fprintf(stdout, "mCurrentClientCount: %d, errorConnect: %d\n", mCurrentClientCount,
            mMaxClientCount - mCurrentClientCount);

    assert(mCurrentClientCount == mPollFDs.size());

    while (mIsRunning) {
        Loop();
    }

    Clear();
}

void PollThread::ConnectClients()
{
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.0.8", &serverAddress.sin_addr);
    serverAddress.sin_port = htons(7799);

    std::vector<WSAPOLLFD> connectPollFDs;
    int blockCount = 0;

    for (int count = 0; count < mMaxClientCount; ++count) {
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            HandleError(__LINE__);
            continue;
        }

        u_long nonblockMode = 0;
        ioctlsocket(clientSocket, FIONBIO, &nonblockMode);

        int result = connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
        if (result == SOCKET_ERROR) {
            int errorCode = WSAGetLastError();
            if (errorCode == WSAEWOULDBLOCK || errorCode == WSAEINPROGRESS) {
                WSAPOLLFD pollFD{};
                pollFD.fd = clientSocket;
                pollFD.events = POLLOUT;
                connectPollFDs.push_back(pollFD);
                blockCount++;
            } else {
                HandleError(__LINE__);
            }
        } else {
            mPollFDs.insert(std::make_pair(clientSocket, new Session(clientSocket)));
            mCurrentClientCount++;
        }
    }

    if (blockCount == 0)
        return;

    int connectResult = 0;
    int loopCount = 0;
    for (; connectResult != blockCount && loopCount <= 5;) {
        ++loopCount;
        int result = WSAPoll(connectPollFDs.data(), connectPollFDs.size(), 1000);
        if (result == SOCKET_ERROR) {
            HandleError(__LINE__);
            continue;
        } else if (result == 0) {
            fprintf(stderr, "timeout\n");
            continue;
        }

        connectResult += result;
        for (int i = 0; i < connectPollFDs.size(); ++i) {
            if (connectPollFDs[i].revents & POLLOUT) {
                int err = 0;
                int len = sizeof(err);
                getsockopt(connectPollFDs[i].fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                if (err == 0) {
                    mCurrentClientCount++;
                    mPollFDs.insert(
                        std::make_pair(connectPollFDs[i].fd, new Session(connectPollFDs[i].fd)));
                } else {
                    fprintf(stderr, "%d] connect error code: %d\n", i, err);
                }
            }
        }
    }
}

} // namespace test