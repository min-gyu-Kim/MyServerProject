#include "core/Scheduler_uring.hpp"
#include <cerrno>
#include <ctime>
#include <errno.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <liburing.h>
#include "core/Container.hpp"
#include "core/Debug.hpp"

namespace core {

namespace {

struct ThreadContext
{
    io_uring mRing;
    // TODO: Queue<typename Task> mTaskQueue;
    // TODO: Have Timer queue
};

__thread ThreadContext sThreadContext;

const Int32 sMAX_ENTRIES = 1024;

bool WaitEvents(io_uring *ring, io_uring_cqe *cqes[], timespec *waitTime)
{
    const Int32 ioResult =
        io_uring_wait_cqe_timeout(ring, cqes, (__kernel_timespec *)waitTime);

    if (ioResult < 0) {
        const Int32 resultErrno = -ioResult;
        if (ETIMEDOUT == resultErrno || EINTR == resultErrno) {
            return false;
        }

        ASSERT(false, fmt::format("uring wait error. error: {}",
                                  strerror(resultErrno)));
        return false;
    }

    return true;
}

void DispatchTaskCompletion(io_uring_cqe *completeEntry)
{
    void *dataPtr = io_uring_cqe_get_data(completeEntry);

    // TODO: convert dataPtr, call complete function ()
    /*
    Task* task = reinterpret_cast<Task*>(dataPtr);

    switch (task->task_type) {
    case ACCEPT:
        // TODO: acceptor notify
        break;
    case CONNECT:
        // TODO: TCP Connector notify
        break;
    case RECV:
        // TODO: TCP Connector notify
        break;
    case SEND:
        // TODO: TCP Connector notify
        break;
    case READ:
        // TODO: File Handler notify
        break;
    case WRITE:
        // TODO: File Handler notify
        break;
    case TIMER:
        // TODO: Timer notify
        break;
    case USER_TASK:
        // TODO: UserTask notify
        break;
    default:
        ASSERT(false,
               fmt::format("{} : task->type is unknown", task->task_type));
        break;
    }
    */
}

} // namespace

SchedulerUring::SchedulerUring()
{
}

SchedulerUring::~SchedulerUring()
{
}

void SchedulerUring::Run()
{
    io_uring &refRing = sThreadContext.mRing;
    timespec waitTimeSpec{1, 0};

    io_uring_queue_init(sMAX_ENTRIES, &refRing, IORING_SETUP_SQPOLL);

    io_uring_cqe *completeEntries[sMAX_ENTRIES]{};
    while (!mStop) {
        if (!WaitEvents(&refRing, completeEntries, &waitTimeSpec)) {
            continue;
        }

        io_uring_cqe *completeEntry = nullptr;
        UInt32 head;
        UInt32 count = 0;
        io_uring_for_each_cqe(&refRing, head, completeEntry)
        {
            DispatchTaskCompletion(completeEntry);

            ++count;
        }

        io_uring_cq_advance(&refRing, count);
    }

    io_uring_queue_exit(&refRing);
}

} // namespace core