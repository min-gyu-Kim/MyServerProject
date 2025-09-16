#include "core/Network/Endpoint.hpp"
#include "core/Debug.hpp"
#include <cstring>

namespace core {
Endpoint::Endpoint(const sockaddr* addr)
{	
	if(addr->sa_family == AF_INET)
	{
		const size_t sockAddrLen = sizeof(sockaddr_in);
		::memcpy(&mAddress, addr, sockAddrLen);
	}
	else if(addr->sa_family == AF_INET6)
	{
		const size_t sockAddrLen = sizeof(sockaddr_in6);
		::memcpy(&mAddress, addr, sockAddrLen);
	}
	else
	{
		ASSERT(false, fmt::format("Unknown address family : {}", addr->sa_family));
	}
}

Endpoint::~Endpoint() {}
}