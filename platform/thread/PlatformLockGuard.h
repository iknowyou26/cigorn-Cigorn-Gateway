#ifndef PLATFORM_LOCK_GUARD_H
#define PLATFORM_LOCK_GUARD_H

#include "PlatformMutex.h"

namespace cigorn
{

class PlatformLockGuard
{
public:
    explicit PlatformLockGuard(PlatformMutex& m)
        : mutex_(m)
    {
        mutex_.lock();
    }

    ~PlatformLockGuard()
    {
        mutex_.unlock();
    }

private:
    PlatformMutex& mutex_;
};

}

#endif