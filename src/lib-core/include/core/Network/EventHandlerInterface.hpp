#pragma once

#include "Endpoint.hpp"

namespace core{
class IAcceptorEvent
{
public:
	virtual bool OnAccept(const Endpoint& clientEndpoint) = 0;
};

class ISessionEvent
{
public:
	virtual void OnConnect(const SessionID clientID) = 0;
	virtual void OnRecv(class Packet*) = 0;
	virtual void OnDisconnect() = 0;
};
}