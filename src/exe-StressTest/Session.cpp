#include "Session.hpp"
#include <stdio.h>
#include <cassert>

namespace test {

namespace {

enum class ePacketID : unsigned short { PING };

struct PacketHeader
{
    unsigned short mSize; // include header size -> packet size = header size (4) + payload size (?)
    ePacketID mID;
};

struct PingPacket
{
    LARGE_INTEGER mSendTick;
};

const int sPACKET_HEADER_SIZE = sizeof(PacketHeader);
const int sPING_PACKET_SIZE = sizeof(PacketHeader) + sizeof(PingPacket);
} // namespace

thread_local LONGLONG sMAX_RTT = 0;
thread_local LONGLONG sRECV_COUNT = 0;
thread_local LONGLONG sSEND_COUNT = 0;

Session::Session(SOCKET clientFD) : mSocket(clientFD)
{
    PacketHeader header{sPING_PACKET_SIZE, ePacketID::PING};
    PingPacket packet{};

    QueryPerformanceCounter(&packet.mSendTick);

    mSendBuffer.Write((char*)&header, sPACKET_HEADER_SIZE);
    mSendBuffer.Write((char*)&packet, sizeof(PingPacket));
}

Session::~Session()
{
    mRecvBuffer.Reset();
    mSendBuffer.Reset();
}

bool Session::DoRecv()
{
    int result = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), mRecvBuffer.GetDirectWriteSize(), 0);
    if (result == 0) {
        closesocket(mSocket);
        fprintf(stderr, "%lld socket recv 0\n", mSocket);
        return false;
    } else if (result == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();
        char errorMessage[256];
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                       errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage,
                       sizeof(errorMessage), nullptr);
        fprintf(stderr, "recv error code : %d (%s)\n", errorCode, errorMessage);
        closesocket(mSocket);
        return false;
    }

    mRecvBuffer.MoveWriteOffset(result);

    do {
        const int bufferUsedSize = mRecvBuffer.GetUsedSize();
        if (bufferUsedSize < sPACKET_HEADER_SIZE) {
            break;
        }

        PacketHeader header;
        int peekSize = mRecvBuffer.Peek((char*)&header, sPACKET_HEADER_SIZE);
        assert(peekSize == sPACKET_HEADER_SIZE);
        assert(header.mID == ePacketID::PING);
        assert(header.mSize == sPING_PACKET_SIZE);

        if (bufferUsedSize < header.mSize) {
            break;
        }

        mRecvBuffer.MoveReadOffset(sPACKET_HEADER_SIZE);
        PingPacket packet;
        const int readSize = mRecvBuffer.Read((char*)&packet, sizeof(PingPacket));
        assert(readSize == sizeof(PingPacket));

        LARGE_INTEGER curTick;
        QueryPerformanceCounter(&curTick);
        LONGLONG interval = curTick.QuadPart - packet.mSendTick.QuadPart;
        sMAX_RTT = max(sMAX_RTT, interval);

        packet.mSendTick = curTick;

        mSendBuffer.Write((char*)&header, sPACKET_HEADER_SIZE);
        mSendBuffer.Write((char*)&packet, sizeof(PingPacket));
    } while (true);

    if (mRecvBuffer.GetUsedSize() == 0) {
        mRecvBuffer.Reset();
    }

    return true;
}

bool Session::DoSend()
{
    if (mSendBuffer.GetUsedSize() == 0) {
        return false;
    }

    const int sendSize = mSendBuffer.GetDIrectReadSize();
    int result = send(mSocket, mSendBuffer.GetFrontBufferPtr(), mSendBuffer.GetDIrectReadSize(), 0);
    if (result == 0) {
        fprintf(stderr, "send size 0\n");
        return false;
    } else if (result == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();
        char errorMessage[256];
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                       errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage,
                       sizeof(errorMessage), nullptr);
        fprintf(stderr, "send error code : %d (%s)\n", errorCode, errorMessage);
        closesocket(mSocket);
        return false;
    }

    mSendBuffer.MoveReadOffset(result);

    if (mSendBuffer.GetUsedSize() == 0) {
        mSendBuffer.Reset();
    }

    return true;
}

} // namespace test