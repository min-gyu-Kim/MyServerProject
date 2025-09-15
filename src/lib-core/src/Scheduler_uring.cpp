#include <cerrno>
#include <ctime>
#include <errno.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <liburing.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#include <poll.h>
#include "core/Container.hpp"
#include "core/Scheduler_uring.hpp"
#include "core/Debug.hpp"

namespace core {

namespace {
const int sMAX_RING_ENTRIES = 1024;

} // namespace

SchedulerUring::SchedulerUring()
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

void SchedulerUring::RegistAcceptor(Acceptor* acceptor)
{
    (void)acceptor;
    /*
    pthread_mutex_lock(&mMutex);
    io_uring_sqe* sqe = io_uring_get_sqe(&mRing);
    if (sqe == nullptr) {
        ASSERT(false, "io_uring_get_sqe() failed");
        // TODO: retry
    }

    io_uring_prep_multishot_accept(sqe, listenFd, nullptr, 0, 0);
    // TODO: accept data
    io_uring_submit(&mRing);
    pthread_mutex_unlock(&mMutex);
    */
}

} // namespace core