#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fmt/core.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

enum class eMessageID : int16_t
{
    EchoMessage = 1,
};

struct MessageHeader
{
    eMessageID mId;
    int16_t mSize;
};

int main()
{
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    const ushort PORT = 8888;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (-1 == connect(clientFd, reinterpret_cast<sockaddr *>(&serverAddr),
                      sizeof(serverAddr))) {
        close(clientFd);
        return 1;
    }

    const int RECV_BUFFER_SIZE = 1024;
    std::array<char, RECV_BUFFER_SIZE> recvBuffer = {};

    while (true) {
        MessageHeader sendMessageHeader{eMessageID::EchoMessage, 0};
        const char *testString = "Hello, World!";
        sendMessageHeader.mSize = static_cast<int16_t>(strlen(testString));
        ssize_t bytesSent =
            send(clientFd, &sendMessageHeader, sizeof(sendMessageHeader), 0);
        if (bytesSent <= 0) {
            close(clientFd);
            return 1;
        }
        bytesSent = send(clientFd, testString, sendMessageHeader.mSize, 0);
        if (bytesSent <= 0) {
            close(clientFd);
            return 1;
        }

        MessageHeader header{};
        ssize_t bytesRead = recv(clientFd, &header, sizeof(header), 0);
        if (bytesRead <= 0) {
            break;
        }

        // Process the message based on the header
        if (header.mSize >= 0 && header.mSize <= recvBuffer.size()) {
            for (ssize_t remainSize = header.mSize; remainSize > 0;) {
                ssize_t bytesReceived = recv(
                    clientFd, recvBuffer.data() + (header.mSize - remainSize),
                    remainSize, 0);
                if (bytesReceived <= 0) {
                    close(clientFd);
                    return 1;
                }
                remainSize -= bytesReceived;
            }

            // Process the received message
            if (header.mId == eMessageID::EchoMessage) {
                fmt::print("Received echo message: {:.*s}\n", header.mSize,
                           recvBuffer.data());
            }
        }
        else {
            // Handle unexpected message or buffer overflow attempt
            fmt::print(stderr,
                       "Unexpected message received or message too large\n");
            break;
        }
    }

    close(clientFd);
    return 0;
}
