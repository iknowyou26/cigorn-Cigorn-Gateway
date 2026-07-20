#ifndef CIGORN_PLATFORM_SOCKET_H
#define CIGORN_PLATFORM_SOCKET_H

#include "platform/Platform.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using CigornSocket = SOCKET;
constexpr CigornSocket CIGORN_INVALID_SOCKET = INVALID_SOCKET;
constexpr int CIGORN_SOCKET_ERROR = SOCKET_ERROR;
constexpr int CIGORN_SHUT_BOTH = SD_BOTH;

inline int CigornCloseSocket(CigornSocket s) { return closesocket(s); }
inline int CigornSetNonBlocking(CigornSocket s, bool enabled)
{
    u_long mode = enabled ? 1UL : 0UL;
    return ioctlsocket(s, FIONBIO, &mode);
}
inline int CigornLastSocketError() { return WSAGetLastError(); }

#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
using CigornSocket = int;
constexpr CigornSocket CIGORN_INVALID_SOCKET = -1;
constexpr int CIGORN_SOCKET_ERROR = -1;
constexpr int CIGORN_SHUT_BOTH = SHUT_RDWR;

inline int CigornCloseSocket(CigornSocket s) { return close(s); }
inline int CigornSetNonBlocking(CigornSocket s, bool enabled)
{
    const int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(s, F_SETFL, enabled ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
}
inline int CigornLastSocketError() { return errno; }
#endif

inline int CigornShutdownSocket(CigornSocket s)
{
    return shutdown(s, CIGORN_SHUT_BOTH);
}

inline int CigornSend(CigornSocket s, const char* buffer, int byteCount)
{
    return send(s, buffer, byteCount, 0);
}

#endif
