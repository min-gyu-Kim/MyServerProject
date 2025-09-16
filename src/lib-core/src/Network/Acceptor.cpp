#include "core/Debug.hpp"
#include "core/Network/Endpoint.hpp"
#include "core/Types.hpp"
#include <cstring>
#include <fmt/base.h>
#include <fmt/format.h>
#include "Acceptor.hpp"

namespace core{

bool Acceptor::Initialize(const AcceptorCreateParameter& param)
{
	ASSERT(mListenFd == -1, "Acceptor already init");

	const bool bCheckInvalidParameter = (nullptr == param.mAcceptHandler)
		|| (nullptr == param.mSessionHandler)
		|| (param.mMaxSessionCount <= 0);
	if(bCheckInvalidParameter)
	{
		fmt::print(stderr, "Acceptor Initialize() invalid parameter");
		return false;
	}
	
	const Endpoint::eIPVersion eIPVersion = param.mEndpoint.GetIPVersion();
	if(eIPVersion == Endpoint::NONE)
	{
		return false;
	}

	const Int32 sockFamily = param.mEndpoint.GetIPVersion() == Endpoint::IPv4 ? AF_INET : AF_INET6;
	
	const SocketFD listenFD = socket(sockFamily, SOCK_STREAM, 0);
	if(sINVALID_SOCKET_FD == listenFD)
	{
		fmt::print(stderr, "Acceptor socket() error code : {}({})", errno, strerror(errno));
		return false;
	}

	if(listen(listenFD, SOMAXCONN) != 0)
	{
		fmt::print(stderr, "Acceptor listen() error code : {}({})", errno, strerror(errno));
		return false;
	}

	mListenFd = listenFD;
	
	ASSERT(mScheduler, "scheduler pointer must be NOT_NULL");
	//mScheduler->RegistAcceptor(this);

    return true;
}

void Acceptor::OnEvent(SocketFD clientFD, const sockaddr* clientAddress) 
{
	(void)clientFD;
	const Endpoint clientEndpoint(clientAddress);
	bool result = false;
	if(nullptr != mEventHandler)
	{
		result = mEventHandler->OnAccept(clientEndpoint);
	}

	if(!result)
	{
		return;
	}

	//mScheduler->RegistSession();
}

}