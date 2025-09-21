#include <core/Server.hpp>

class TestServer : public core::Server
{
  public:
    bool OnAccepeted(const core::Endpoint& clientEndpoint) override
    {
        char ipAddressString[30]{};
        clientEndpoint.GetIPAddressString(ipAddressString, 30);
        printf("OnAccept! ip: %s, port: %u\n", ipAddressString, clientEndpoint.GetPort());

        return true;
    }

    void OnConnected(const core::SessionID sessionID) override
    {
        printf("OnConnected! sessionID %lld\n", sessionID);
    }

    core::Int32 OnRecv(core::SessionID sessionID, const core::Byte* buffer,
                       core::Int32 inputSize) override
    {
        Send(sessionID, buffer, inputSize);
        printf("OnRecv and send. size : %d\n", inputSize);

        return inputSize;
    }

    void OnDisconnected(core::SessionID sessionID) override
    {
        printf("OnDieconnectd. sessionID : %lld\n", sessionID);
    }
};

int main()
{
    TestServer server;
    core::Endpoint serverEndpoint(core::Endpoint::IPv4, "0.0.0.0", 7799);
    if (!server.Start(serverEndpoint, 12)) {
        fprintf(stderr, "server start failed");
        return -1;
    }

    while (true) {
        sleep(-1);
    }

    server.Stop();

    return 0;
}
