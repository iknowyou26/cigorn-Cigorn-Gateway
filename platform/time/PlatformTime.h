#ifndef CIGORN_PLATFORM_TIME_H
#define CIGORN_PLATFORM_TIME_H

#include <chrono>
#include <cstdint>
#include <thread>

namespace cigorn
{
inline void SleepMilliseconds(std::uint64_t value)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(value));
}

inline void SleepMicroseconds(std::uint64_t value)
{
    std::this_thread::sleep_for(std::chrono::microseconds(value));
}

inline std::int64_t UnixTimeMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}
}

#endif
