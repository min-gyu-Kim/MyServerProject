#include <cerrno>
#include <ctime>
#include <fmt/core.h>
#include <fmt/format.h>
#include <liburing.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <poll.h>
#include "core/Scheduler_uring.hpp"
#include "core/Debug.hpp"
#include "Network/Acceptor.hpp"

namespace core {

namespace {
const int sMAX_RING_ENTRIES = 1024;

enum class eEventType : UInt8 { ACCEPT, RECV, SEND, };

} // namespace

SchedulerUring::SchedulerUring() : mStop(true)
{
    pthread_mutex_init(&mMutex, nullptr);
    if (io_uring_queue_init(sMAX_RING_ENTRIES, &mRing, 0) < 0) {
        ASSERT(false, "io_uring_queue_init() failed");
    }
}

SchedulerUring::~SchedulerUring()
{
}

void SchedulerUring::Run()
{
    while (mStop) {
    }
}

AcceptorID SchedulerUring::CreateAcceptor(const AcceptorCreateParameter& param)
{
    Acceptor* acceptor = new Acceptor(this);
    if(!acceptor->Initialize(param))
    {
        return sINVALID_ID;
    }

    pthread_mutex_lock(&mMutex);
    io_uring_sqe* sqe = io_uring_get_sqe(&mRing);
    if(!sqe)
    {
        const Int32 result = io_uring_submit(&mRing);
        if(result < 0)
        {
            fmt::print(stderr, "io_uring_submit error: {} ({})", -result, strerror(-result));
            return sINVALID_ID;
        }
    }
    pthread_mutex_unlock(&mMutex);

    return sINVALID_ID;
}

} // namespace core