#include "PlatformSocket.h"

namespace cigorn
{

bool PlatformSocket::Initialize()
{
#ifdef _WIN32
    WSADATA data;
    return WSAStartup(MAKEWORD(2,2), &data) == 0;
#else
    return true;
#endif
}

void PlatformSocket::Shutdown()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

void PlatformSocket::Close(SocketHandle socket)
{
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

}