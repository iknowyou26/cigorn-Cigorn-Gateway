#ifndef PLATFORM_SOCKET_H
#define PLATFORM_SOCKET_H

#include "../Platform.h"

namespace cigorn
{

class PlatformSocket
{
public:
#ifdef _WIN32
    using SocketHandle = SOCKET;
    static constexpr SocketHandle InvalidSocket = INVALID_SOCKET;
#else
    using SocketHandle = int;
    static constexpr SocketHandle InvalidSocket = -1;
#endif

    static bool Initialize();
    static void Shutdown();

    static void Close(SocketHandle socket);

private:
    PlatformSocket() = delete;
};

}

#endif