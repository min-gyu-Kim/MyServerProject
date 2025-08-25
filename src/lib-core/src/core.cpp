#include "core/core.hpp"
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <fmt/core.h>
#include <stdexcept>

namespace core {
namespace {

enum class eIOType : std::uint8_t
{
    Accept,
    Connect,
    Recv,
    Send
};

struct IORequest
{
    eIOType mType;
    int mFd;        // File descriptor
    void *mBuffer;  // Buffer for data
    size_t mLength; // Length of data
};

} // anonymous namespace

Scheduler::Scheduler() : mRing{}
{
    const int QUEUE_SIZE = 256;
    int result = io_uring_queue_init(QUEUE_SIZE, &mRing, 0);
    if (result < 0) {
        throw std::runtime_error(fmt::format(
            "Failed to initialize io_uring: {}", strerror(-result)));
    }

    mListenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mListenFd < 0) {
        throw std::runtime_error(
            fmt::format("Failed to create socket: {}", strerror(errno)));
    }

    int optval = 1;
    if (setsockopt(mListenFd, SOL_SOCKET, SO_REUSEPORT, &optval,
                   sizeof(optval)) < 0) {
        perror("setsockopt SO_REUSEPORT failed");
        close(mListenFd);
        return;
    }

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    const ushort PORT = 8888;
    listenAddr.sin_port = htons(PORT);

    if (bind(mListenFd, reinterpret_cast<struct sockaddr *>(&listenAddr),
             sizeof(listenAddr)) < 0) {
        throw std::runtime_error(
            fmt::format("Failed to bind socket: {}", strerror(errno)));
    }

    if (listen(mListenFd, SOMAXCONN) < 0) {
        throw std::runtime_error(
            fmt::format("Failed to listen on socket: {}", strerror(errno)));
    }

    io_uring_sqe *sqe = io_uring_get_sqe(&mRing);
    if (nullptr == sqe) {
        throw std::runtime_error(fmt::format(
            "Failed to get submission queue entry: {}", strerror(errno)));
    }

    sockaddr_in *addrPtr = new sockaddr_in;
    IORequest *request =
        new IORequest{eIOType::Accept, mListenFd, addrPtr, sizeof(sockaddr_in)};

    sqe->user_data = reinterpret_cast<unsigned long long>(request);

    io_uring_prep_accept(sqe, mListenFd,
                         reinterpret_cast<struct sockaddr *>(addrPtr),
                         reinterpret_cast<socklen_t *>(&request->mLength), 0);
    result = io_uring_submit(&mRing);
    assert(result >= 0 && "Failed to submit accept request");
}

Scheduler::~Scheduler()
{
    io_uring_queue_exit(&mRing);
}

void Scheduler::Run()
{
    const int MAX_CQES = 32;
    while (true) {
        std::array<io_uring_cqe *, MAX_CQES> cqes = {};
        __kernel_timespec timespec = {1, 0}; // No timeout
        int result = io_uring_wait_cqes(&mRing, cqes.data(), MAX_CQES,
                                        &timespec, nullptr);
        if (result < 0) {
            if (-ETIME == result) {
                // Handle timeout case
                continue;
            }
            throw std::runtime_error(fmt::format(
                "Failed to wait for completion: {}", strerror(-result)));
        }

        const unsigned int EVENT_COUNT =
            io_uring_peek_batch_cqe(&mRing, cqes.data(), MAX_CQES);
        for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
            io_uring_cqe *cqe = cqes.at(i);

            if (cqe->res < 0) {
                throw std::runtime_error(fmt::format("IO submission failed: {}",
                                                     strerror(-cqe->res)));
            }

            IORequest *request =
                reinterpret_cast<IORequest *>(io_uring_cqe_get_data(cqe));
            switch (request->mType) {
            case eIOType::Accept:
            {
                /* code */
                OnAccept(cqe->res, request->mBuffer, request->mLength);

                io_uring_sqe *sqe = io_uring_get_sqe(&mRing);
                if (nullptr == sqe) {
                    io_uring_submit(&mRing);
                    sqe = io_uring_get_sqe(&mRing);
                }
                sqe->user_data = reinterpret_cast<unsigned long long>(request);
                io_uring_prep_accept(
                    sqe, request->mFd,
                    reinterpret_cast<sockaddr *>(request->mBuffer),
                    reinterpret_cast<socklen_t *>(&request->mLength), 0);
            } break;
            case eIOType::Connect:
                /* code */
                break;
            case eIOType::Recv:
                /* code */
                OnRecv(request, cqe->res);
                break;
            case eIOType::Send:
                /* code */
                break;
            default:
                throw std::runtime_error(fmt::format(
                    "Unknown IO type: {}", static_cast<int>(request->mType)));
                break;
            }

            // Process the completion event
            io_uring_cqe_seen(&mRing, cqe);
        }

        io_uring_submit(&mRing);
    }
}

void Scheduler::OnAccept(int clientFd, void *buffer, size_t length)
{
    (void)length; // Unused in this context
    // Handle the accept event
    sockaddr_in *addrPtr = reinterpret_cast<sockaddr_in *>(buffer);
    std::array<char, INET_ADDRSTRLEN> ipAddress = {};
    if (nullptr == inet_ntop(AF_INET, &addrPtr->sin_addr, ipAddress.data(),
                             INET_ADDRSTRLEN)) {
        throw std::runtime_error(
            fmt::format("Failed to convert IP address: {}", strerror(errno)));
    }
    unsigned short port = ntohs(addrPtr->sin_port);

    fmt::print("Accepted connection from {}:{}", ipAddress.data(), port);

    const int RECV_BUFFER_SIZE = 1500;
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    char *recvBuffer = new char[RECV_BUFFER_SIZE];
    IORequest *request =
        new IORequest{eIOType::Recv, clientFd, recvBuffer, RECV_BUFFER_SIZE};
    // NOLINTEND(cppcoreguidelines-owning-memory)

    io_uring_sqe *sqe = io_uring_get_sqe(&mRing);
    if (nullptr == sqe) {
        io_uring_submit(&mRing);
        sqe = io_uring_get_sqe(&mRing);
    }
    sqe->user_data = reinterpret_cast<unsigned long long>(request);
    io_uring_prep_recv(sqe, clientFd, recvBuffer, RECV_BUFFER_SIZE, 0);
}

void Scheduler::OnRecv(void *request, int transferred)
{
    // Handle the receive event
    IORequest *req = static_cast<IORequest *>(request);

    if (transferred <= 0) {
        fmt::print("Client {} disconnected or error occurred: {}", req->mFd,
                   transferred);
        delete[] static_cast<char *>(req->mBuffer);
        delete req;
        return;
    }

    // NOLINTBEGIN(performance-enum-size)
    enum class eMessageID : int16_t
    {
        EchoMessage = 1,
    };
    // NOLINTEND(performance-enum-size)

    struct MessageHeader
    {
        eMessageID mId;
        int16_t mSize;
        char mData[0]; // Flexible array member
    };

    MessageHeader *header = static_cast<MessageHeader *>(req->mBuffer);
    fmt::print("Received data from client {}: {}", req->mFd,
               std::string(header->mData, header->mSize));

    if (transferred != header->mSize) {
        fmt::print(
            "Received incomplete packet from client {}: expected {}, got {}",
            req->mFd, header->mSize, transferred);
    }

    io_uring_sqe *sqe = io_uring_get_sqe(&mRing);
    if (nullptr == sqe) {
        io_uring_submit(&mRing);
        sqe = io_uring_get_sqe(&mRing);
    }
    sqe->user_data = reinterpret_cast<unsigned long long>(req);
    io_uring_prep_recv(sqe, req->mFd, req->mBuffer, req->mLength, 0);
}

} // namespace core
