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

    void OnDisconnected(core::SessionID sessionID) override
    {
    }
};

int main()
{
    TestServer server;
    core::Endpoint serverEndpoint(core::Endpoint::IPv4, "0.0.0.0", 7799);
    if (!server.Start(serverEndpoint, 4)) {
        fprintf(stderr, "server start failed");
        return -1;
    }

    while (true) {
        sleep(-1);
    }

    server.Stop();

    return 0;
}
