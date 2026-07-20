#ifndef CIGORN_PLATFORM_PROCESS_H
#define CIGORN_PLATFORM_PROCESS_H

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace cigorn
{
inline int CurrentProcessId()
{
#ifdef _WIN32
    return _getpid();
#else
    return static_cast<int>(getpid());
#endif
}
}

#endif
