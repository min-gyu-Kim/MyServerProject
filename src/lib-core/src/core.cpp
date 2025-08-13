#include <stdexcept>
#include <fmt/core.h>
#include <arpa/inet.h>
#include "libcore/core.hpp"

namespace core {
namespace{

enum class eIOType{
    ACCEPT,
    CONNECT,
    RECV,
    SEND
};

struct IORequest {
    eIOType type;
    int fd; // File descriptor
    void* buffer; // Buffer for data
    size_t length; // Length of data
};

} // anonymous namespace


Scheduler::Scheduler()
{
    int result = io_uring_queue_init(256, &m_ring, 0);
    if(result < 0)
    {
        throw std::runtime_error(fmt::format("Failed to initialize io_uring: {}", strerror(-result)));
    }

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_listenFd < 0)
    {
        throw std::runtime_error(fmt::format("Failed to create socket: {}", strerror(errno)));
    }

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    listenAddr.sin_port = htons(8888);

    if(bind(m_listenFd, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0)
    {
        throw std::runtime_error(fmt::format("Failed to bind socket: {}", strerror(errno)));
    }

    if(listen(m_listenFd, SOMAXCONN) < 0)
    {
        throw std::runtime_error(fmt::format("Failed to listen on socket: {}", strerror(errno)));
    }

    io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
    if(!sqe)
    {
        throw std::runtime_error(fmt::format("Failed to get submission queue entry: {}", strerror(errno)));
    }

    sockaddr_in* addrPtr = new sockaddr_in;
    IORequest* request = new IORequest{
        eIOType::ACCEPT,
        m_listenFd,
        addrPtr,
        sizeof(sockaddr_in)
    };
    sqe->user_data = (unsigned long long)request;

    io_uring_prep_accept(sqe, m_listenFd, (struct sockaddr*)addrPtr, (socklen_t*)&request->length, 0);
}

Scheduler::~Scheduler()
{
    io_uring_queue_exit(&m_ring);
}

void Scheduler::Run()
{
    while(true)
    {
        io_uring_cqe* cqes[32];
        __kernel_timespec ts = {1, 0}; // No timeout
        int result = io_uring_wait_cqes(&m_ring, cqes, 32, &ts, nullptr);
        if(result < 0)
        {
            if(-ETIME == result)
            {
                // Handle timeout case
                continue;
            }
            throw std::runtime_error(fmt::format("Failed to wait for completion: {}", strerror(-result)));
        }

        for(int i = 0; i < result; ++i)
        {
            io_uring_cqe* cqe = cqes[i];

            if(cqe->res < 0)
            {
                throw std::runtime_error(fmt::format("IO submission failed: {}", strerror(-cqe->res)));
            }

            IORequest* request = (IORequest*)cqe->user_data;
            switch (request->type)
            {
            case eIOType::ACCEPT:
            {
                /* code */
                OnAccept(request->fd, request->buffer, request->length);

                io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
                if(nullptr == sqe)
                {
                    io_uring_submit(&m_ring);
                    sqe = io_uring_get_sqe(&m_ring);
                }
                sqe->user_data = (unsigned long long)request;
                io_uring_prep_accept(sqe, request->fd, (sockaddr*)request->buffer, (socklen_t*)&request->length, 0);
            }
                break;
            case eIOType::CONNECT:
                /* code */
                break;
            case eIOType::RECV:
                /* code */
                OnRecv(request, cqe->res);
                break;
            case eIOType::SEND:
                /* code */
                break;
            default:
                throw std::runtime_error(fmt::format("Unknown IO type: {}", static_cast<int>(request->type)));
                break;
            }

            // Process the completion event
            io_uring_cqe_seen(&m_ring, cqe);
        }

        io_uring_submit(&m_ring);
    }
}

void Scheduler::OnAccept(int clientFd, void* buffer, size_t length)
{
    (void)length; // Unused in this context
    // Handle the accept event
    sockaddr_in* addrPtr = (sockaddr_in*)buffer;
    char ip[INET_ADDRSTRLEN];
    if(nullptr == inet_ntop(AF_INET, &addrPtr->sin_addr, ip, INET_ADDRSTRLEN))
    {
        throw std::runtime_error(fmt::format("Failed to convert IP address: {}", strerror(errno)));
    }
    unsigned short port = ntohs(addrPtr->sin_port);

    fmt::print("Accepted connection from {}:{}", ip, port);

    char* recvBuffer = new char[1500];
    IORequest* request = new IORequest{
        eIOType::RECV,
        clientFd,
        recvBuffer,
        1500
    };

    io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
    if(nullptr == sqe)
    {
        io_uring_submit(&m_ring);
        sqe = io_uring_get_sqe(&m_ring);
    }
    sqe->user_data = (unsigned long long)request;
    io_uring_prep_recv(sqe, clientFd, recvBuffer, 1500, 0);
 }

void Scheduler::OnRecv(void* request, int transferred)
{
    // Handle the receive event
    IORequest* req = static_cast<IORequest*>(request);

    if(transferred <= 0)
    {
        fmt::print("Client {} disconnected or error occurred: {}", req->fd, transferred);
        delete[] static_cast<char*>(req->buffer);
        delete req;
        return;
    }

    char* data = static_cast<char*>(req->buffer);
    fmt::print("Received data from client {}: {}", req->fd, std::string(data, transferred));

    short* packetSize = (short*)data;
    if(transferred != *packetSize)
    {
        fmt::print("Received incomplete packet from client {}: expected {}, got {}", req->fd, *packetSize, transferred);
    }

    io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
    if(nullptr == sqe)
    {
        io_uring_submit(&m_ring);
        sqe = io_uring_get_sqe(&m_ring);
    }
    sqe->user_data = (unsigned long long)req;
    io_uring_prep_recv(sqe, req->fd, req->buffer, req->length, 0);
}

} // namespace core
