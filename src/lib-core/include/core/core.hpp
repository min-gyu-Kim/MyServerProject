#pragma once

#include <liburing.h>

namespace core {

class Scheduler
{
  public:
    Scheduler();
    ~Scheduler();

    void Run();

  private:
    void OnAccept(int clientFd, void *buffer, size_t length);
    void OnRecv(void *request, int transferred);

  private:
    io_uring mRing;
    int mListenFd;
};

} // namespace core
