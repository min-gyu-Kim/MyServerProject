#pragma once

#include "core/Types.hpp"
#include "RecvBuffer.hpp"

namespace core{

class Server;

class Session{
public:
	Session() = delete;
	Session(Server* server, SessionID sessionID, SocketFD sessionFD) :
		mServer(server), mSessionID(sessionID), mSessionFD(sessionFD), mIsSending(false)
	{
		mRecvBuffer = new RecvBuffer();
	}

	~Session()
	{
		delete mRecvBuffer;
	}

	SessionID GetSessionID() const { return mSessionID; }
	SocketFD GetSocketFD() const { return mSessionFD; }
	RecvBuffer* GetRecvBuffer() { return mRecvBuffer; }

private:
	SessionID mSessionID;
	SocketFD mSessionFD;
	RecvBuffer* mRecvBuffer;

	bool mIsSending;
	//class SendPool* ;

	Server* mServer;
};
}