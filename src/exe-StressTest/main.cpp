#include <iostream>
#include <WinSock2.h>
#include "PollThread.hpp"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSAData data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        fprintf(stderr, "WSAStartup failed");
        return 1;
    }

    test::PollThread* pollThread = new test::PollThread(500);
    pollThread->Create();

    Sleep(30000);

    pollThread->Stop();
    delete pollThread;

    WSACleanup();

    return 0;
}