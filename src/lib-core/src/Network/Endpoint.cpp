#include "core/Network/Endpoint.hpp"
#include "core/Debug.hpp"

namespace core {
Endpoint::Endpoint(const sockaddr* addr)
{
    if (addr->sa_family == AF_INET) {
        const size_t sockAddrLen = sizeof(sockaddr_in);
        ::memcpy(&mAddress, addr, sockAddrLen);
    } else if (addr->sa_family == AF_INET6) {
        const size_t sockAddrLen = sizeof(sockaddr_in6);
        ::memcpy(&mAddress, addr, sockAddrLen);
    } else {
        ASSERT(false, fmt::format("Unknown address family : {}", addr->sa_family));
    }
}

Endpoint::Endpoint(eIPVersion ipVersion, const String& ipAddress, UInt16 port)
{
    Int32 addressFamily = AF_UNSPEC;
    if (ipVersion == IPv4) {
        addressFamily = AF_INET;
    } else if (ipVersion == IPv6) {
        addressFamily = AF_INET6;
    } else {
        ASSERT(false, "Invalid IP version");
        return;
    }

    Int32 result =
        inet_pton(addressFamily, ipAddress.c_str(),
                  (addressFamily == AF_INET)
                      ? static_cast<void*>(&reinterpret_cast<sockaddr_in*>(&mAddress)->sin_addr)
                      : static_cast<void*>(&reinterpret_cast<sockaddr_in6*>(&mAddress)->sin6_addr));

    if (result != 1) {
        ASSERT(false,
               fmt::format("inet_pton() failed. addressFamily: {}, ipAddress: {}, result: {}",
                           addressFamily, ipAddress, result));
        return;
    }

    if (addressFamily == AF_INET) {
        sockaddr_in* addrIn = reinterpret_cast<sockaddr_in*>(&mAddress);
        addrIn->sin_family = AF_INET;
        addrIn->sin_port = htons(port);
    } else if (addressFamily == AF_INET6) {
        sockaddr_in6* addrIn6 = reinterpret_cast<sockaddr_in6*>(&mAddress);
        addrIn6->sin6_family = AF_INET6;
        addrIn6->sin6_port = htons(port);
    }
}

Endpoint::~Endpoint()
{
}
} // namespace core