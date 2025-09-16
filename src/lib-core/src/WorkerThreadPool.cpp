#include "WorkerThreadPool.hpp"
#include "core/Debug.hpp"

#include <sys/eventfd.h>

namespace core {

namespace {
typedef struct TagThreadParameter
{
    WorkerThreadPool* mPool;
    Int32 mIndex;
} ThreadParameter;

} // namespace

class Job
{
  public:
    void Execute()
    {
    }
};

void* WorkerThreadPool::WorkerThreadFunc(void* arg)
{
    ThreadParameter* param = static_cast<ThreadParameter*>(arg);
    ASSERT(param->mPool != nullptr, "param->mPool is null.");
    param->mPool->PollEvents(param->mIndex);
    delete param;
    return nullptr;
}

bool WorkerThreadPool::Start(size_t workerThreadCount)
{
    mIsRunning = true;

    mWorkerThreads.resize(workerThreadCount);

    for (size_t i = 0; i < workerThreadCount; ++i) {
        ThreadParameter* param = new ThreadParameter();
        param->mPool = this;
        param->mIndex = static_cast<Int32>(i);

        mWorkerThreads[i].mEventFD = eventfd(0, 0);

        int ret = pthread_create(&mWorkerThreads[i].mThreadHandle, nullptr,
                                 &WorkerThreadPool::WorkerThreadFunc, param);
        if (ret != 0) {
            ASSERT(false, fmt::format("pthread_create() failed. ret: {}", ret));
            return false;
        }
    }

    return true;
}

void WorkerThreadPool::Stop()
{
    mIsRunning = false;
    for (auto& threadContext : mWorkerThreads) {

        pthread_join(threadContext.mThreadHandle, nullptr);
    }
    mWorkerThreads.clear();
}

void WorkerThreadPool::PollEvents(Int32 index)
{
    ThreadContext& threadContext = mWorkerThreads[index];

    while (mIsRunning) {
        Job* job = PopJob();
        if (nullptr == job) {
            Idle(threadContext);
            continue;
        }

        job->Execute();
    }
}

void WorkerThreadPool::WakeOneThread()
{
    if (mIdleThreads.empty()) {
        return;
    }

    ThreadContext context;
    {
        LockGuard<Mutex> lock(mStackMutex);
        context = mIdleThreads.top();
        mIdleThreads.pop();
    }

    UInt64 value = 1;
    ssize_t ret = write(context.mEventFD, &value, sizeof(value));
    if (ret < 0) {
        ASSERT(false, "write() failed.");
    }
}

bool WorkerThreadPool::Idle(const ThreadContext& context)
{
    {
        LockGuard<Mutex> lock(mStackMutex);
        mIdleThreads.push(context);
    }

    Int32 eventFD = context.mEventFD;
    UInt64 value = 0;
    ssize_t ret = read(eventFD, &value, sizeof(value));
    if (ret < 0) {
        ASSERT(false, "read() failed.");
        return false;
    }

    return true;
}

Job* WorkerThreadPool::PopJob()
{
    return nullptr;
}

} // namespace core