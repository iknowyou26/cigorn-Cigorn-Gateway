#ifndef CIGORN_PLATFORM_H
#define CIGORN_PLATFORM_H

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#else

#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

#endif // _WIN32

#endif // CIGORN_PLATFORM_H