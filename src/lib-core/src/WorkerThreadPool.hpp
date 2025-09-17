#pragma once

#include <pthread.h>
#include "core/Container.hpp"
#include "core/Lock.hpp"

namespace core {

class IJob;

class WorkerThreadPool
{
  private:
    typedef struct TagThreadContext
    {
        pthread_t mThreadHandle;
        Int32 mEventFD;
    } ThreadContext;

  public:
    WorkerThreadPool() = default;
    ~WorkerThreadPool() = default;

    bool Start(size_t workerThreadCount);
    void Stop();

    void AddJob(IJob* job);

  private:
    static void* WorkerThreadFunc(void* arg);
    void PollEvents(Int32 index);
    void WakeOneThread();
    bool Idle(const ThreadContext& context);
    IJob* PopJob();

  private:
    bool mIsRunning = false;
    Vector<ThreadContext> mWorkerThreads;

    Mutex mStackMutex;
    Stack<ThreadContext> mIdleThreads;

    Mutex mQueueMutex;
    Queue<IJob*> mJobQueue;
};
} // namespace core