#pragma once

#ifdef __linux__
#include <arpa/inet.h>
#elif _WIN32
#include <winsock2.h>
#endif

#include "../Types.hpp"
#include "../Debug.hpp"

namespace core {

class Endpoint
{
  public:
    enum eIPVersion { NONE, IPv4, IPv6 };

  public:
    Endpoint(const sockaddr* addr);
    ~Endpoint();

    const sockaddr* GetAddress() const
    {
        return reinterpret_cast<const sockaddr*>(&mAddress);
    }

    eIPVersion GetIPVersion() const
    {
        if (mAddress.ss_family == AF_INET) {
            return IPv4;
        } else if (mAddress.ss_family == AF_INET6) {
            return IPv6;
        }

        ASSERT(false, "Unknown address family");
        return NONE;
    }

    Int32 GetIPAddressString(char* buffer, size_t bufferLen) const
    {
        if (mAddress.ss_family == AF_INET) {
            const sockaddr_in* addrIn = reinterpret_cast<const sockaddr_in*>(&mAddress);
            if (inet_ntop(AF_INET, &addrIn->sin_addr, buffer, static_cast<socklen_t>(bufferLen)) ==
                nullptr) {
                return -1;
            }
            return 0;
        } else if (mAddress.ss_family == AF_INET6) {
            const sockaddr_in6* addrIn6 = reinterpret_cast<const sockaddr_in6*>(&mAddress);
            if (inet_ntop(AF_INET6, &addrIn6->sin6_addr, buffer,
                          static_cast<socklen_t>(bufferLen)) == nullptr) {
                return -1;
            }
            return 0;
        }

        ASSERT(false, "Unknown address family");
        return -1;
    }

    UInt16 GetPort() const
    {
        if (mAddress.ss_family == AF_INET) {
            const sockaddr_in* addr_in = reinterpret_cast<const sockaddr_in*>(&mAddress);
            return ntohs(addr_in->sin_port);
        } else if (mAddress.ss_family == AF_INET6) {
            const sockaddr_in6* addr_in6 = reinterpret_cast<const sockaddr_in6*>(&mAddress);
            return ntohs(addr_in6->sin6_port);
        } else {
            ASSERT(false, "Unknown address family");
            return 0;
        }
    }

  private:
    sockaddr_storage mAddress;
};

} // namespace core