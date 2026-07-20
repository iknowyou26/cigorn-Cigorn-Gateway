#pragma once

#include <thread>
#include <utility>

namespace cigorn
{

class PlatformThread
{
public:
    PlatformThread() = default;

    template<typename Function, typename... Args>
    explicit PlatformThread(Function&& function, Args&&... args)
        : thread_(
            std::forward<Function>(function),
            std::forward<Args>(args)...
        )
    {
    }

    PlatformThread(const PlatformThread&) = delete;
    PlatformThread& operator=(const PlatformThread&) = delete;

    PlatformThread(PlatformThread&&) noexcept = default;
    PlatformThread& operator=(PlatformThread&&) noexcept = default;

    ~PlatformThread()
    {
        // The legacy pthread code did not join these worker threads.
        // Detach here to preserve that behavior safely.
        if (thread_.joinable())
            thread_.detach();
    }

    bool joinable() const noexcept
    {
        return thread_.joinable();
    }

    void join()
    {
        if (thread_.joinable())
            thread_.join();
    }

    void detach()
    {
        if (thread_.joinable())
            thread_.detach();
    }

private:
    std::thread thread_;
};

}
