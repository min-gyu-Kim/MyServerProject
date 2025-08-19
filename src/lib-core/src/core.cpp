#include "libcore/core.hpp"
#include <arpa/inet.h>
#include <fmt/core.h>
#include <gsl/gsl>
#include <stdexcept>

namespace core {
namespace {

enum class eIOType
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

Scheduler::Scheduler() : m_ring{}
{
    const int QUEUE_SIZE = 256;
    int result = io_uring_queue_init(QUEUE_SIZE, &m_ring, 0);
    if (result < 0) {
        throw std::runtime_error(fmt::format(
            "Failed to initialize io_uring: {}", strerror(-result)));
    }

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) {
        throw std::runtime_error(
            fmt::format("Failed to create socket: {}", strerror(errno)));
    }

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    const ushort PORT = 8888;
    listenAddr.sin_port = htons(PORT);

    if (bind(m_listenFd, reinterpret_cast<struct sockaddr *>(&listenAddr),
             sizeof(listenAddr)) < 0) {
        throw std::runtime_error(
            fmt::format("Failed to bind socket: {}", strerror(errno)));
    }

    if (listen(m_listenFd, SOMAXCONN) < 0) {
        throw std::runtime_error(
            fmt::format("Failed to listen on socket: {}", strerror(errno)));
    }

    io_uring_sqe *sqe = io_uring_get_sqe(&m_ring);
    if (nullptr == sqe) {
        throw std::runtime_error(fmt::format(
            "Failed to get submission queue entry: {}", strerror(errno)));
    }

    gsl::owner<sockaddr_in *> addrPtr = new sockaddr_in;
    gsl::owner<IORequest *> request = new IORequest{
        eIOType::Accept, m_listenFd, addrPtr, sizeof(sockaddr_in)};

    sqe->user_data = reinterpret_cast<unsigned long long>(request);

    io_uring_prep_accept(sqe, m_listenFd,
                         reinterpret_cast<struct sockaddr *>(addrPtr),
                         reinterpret_cast<socklen_t *>(&request->mLength), 0);
}

Scheduler::~Scheduler()
{
    io_uring_queue_exit(&m_ring);
}

void Scheduler::Run()
{
    const int MAX_CQES = 32;
    while (true) {
        std::array<io_uring_cqe *, MAX_CQES> cqes = {};
        __kernel_timespec timespec = {1, 0}; // No timeout
        int result = io_uring_wait_cqes(&m_ring, cqes.data(), MAX_CQES,
                                        &timespec, nullptr);
        if (result < 0) {
            if (-ETIME == result) {
                // Handle timeout case
                continue;
            }
            throw std::runtime_error(fmt::format(
                "Failed to wait for completion: {}", strerror(-result)));
        }

        for (int i = 0; i < result; ++i) {
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
                OnAccept(request->mFd, request->mBuffer, request->mLength);

                io_uring_sqe *sqe = io_uring_get_sqe(&m_ring);
                if (nullptr == sqe) {
                    io_uring_submit(&m_ring);
                    sqe = io_uring_get_sqe(&m_ring);
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
            io_uring_cqe_seen(&m_ring, cqe);
        }

        io_uring_submit(&m_ring);
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
    gsl::owner<IORequest *> request =
        new IORequest{eIOType::Recv, clientFd, recvBuffer, RECV_BUFFER_SIZE};
    // NOLINTEND(cppcoreguidelines-owning-memory)

    io_uring_sqe *sqe = io_uring_get_sqe(&m_ring);
    if (nullptr == sqe) {
        io_uring_submit(&m_ring);
        sqe = io_uring_get_sqe(&m_ring);
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
        // NOLINTBEGIN(cppcoreguidelines-owning-memory)
        delete[] static_cast<char *>(req->mBuffer);
        delete req;
        // NOLINTEND(cppcoreguidelines-owning-memory)
        return;
    }

    char *data = static_cast<char *>(req->mBuffer);
    fmt::print("Received data from client {}: {}", req->mFd,
               std::string(data, transferred));

    short *packetSize = reinterpret_cast<short *>(data);
    if (transferred != *packetSize) {
        fmt::print(
            "Received incomplete packet from client {}: expected {}, got {}",
            req->mFd, *packetSize, transferred);
    }

    io_uring_sqe *sqe = io_uring_get_sqe(&m_ring);
    if (nullptr == sqe) {
        io_uring_submit(&m_ring);
        sqe = io_uring_get_sqe(&m_ring);
    }
    sqe->user_data = reinterpret_cast<unsigned long long>(req);
    io_uring_prep_recv(sqe, req->mFd, req->mBuffer, req->mLength, 0);
}

} // namespace core
