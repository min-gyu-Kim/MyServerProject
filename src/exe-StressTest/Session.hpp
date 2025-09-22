#pragma once

#include <WinSock2.h>
#include "RingBuffer.hpp"

namespace test {
class Session
{
  private:
    friend class PollThread;

  public:
    Session(SOCKET clientFD);
    ~Session();

    bool DoRecv();
    bool DoSend();

  private:
    SOCKET mSocket;
    RingBuffer mRecvBuffer;
    RingBuffer mSendBuffer;
};
} // namespace test