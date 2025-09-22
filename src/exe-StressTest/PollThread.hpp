#pragma once

#include <WinSock2.h>
#include <vector>
#include <unordered_map>

namespace test {
class Session;

class PollThread
{
  public:
    PollThread();
    PollThread(int clientCount);
    ~PollThread();

    void Create();
    void Loop();
    void Clear();
    void Stop();

  private:
    static unsigned int __stdcall PollThreadFunction(void*);
    void Run();

    void ConnectClients();

  private:
    bool mIsRunning;
    int mMaxClientCount;
    int mCurrentClientCount;
    HANDLE mThreadHandle;
    std::unordered_map<SOCKET, Session*> mPollFDs;
};
} // namespace test