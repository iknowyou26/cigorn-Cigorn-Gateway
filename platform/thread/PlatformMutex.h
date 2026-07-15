#ifndef PLATFORM_MUTEX_H
#define PLATFORM_MUTEX_H

#include <mutex>

namespace cigorn
{

class PlatformMutex
{
public:
    void lock()
    {
        mutex_.lock();
    }

    void unlock()
    {
        mutex_.unlock();
    }

    bool try_lock()
    {
        return mutex_.try_lock();
    }

private:
    std::mutex mutex_;
};

}

#endif