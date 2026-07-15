#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

#include <thread>
#include <utility>

namespace cigorn
{

class PlatformThread
{
public:
    PlatformThread() = default;

    PlatformThread(const PlatformThread&) = delete;
    PlatformThread& operator=(const PlatformThread&) = delete;

    PlatformThread(PlatformThread&&) noexcept = default;
    PlatformThread& operator=(PlatformThread&&) noexcept = default;

    template<typename Function, typename... Args>
    explicit PlatformThread(
        Function&& function,
        Args&&... args
    )
        : thread_(
            std::forward<Function>(function),
            std::forward<Args>(args)...
        )
    {
    }

    ~PlatformThread()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    bool Joinable() const noexcept
    {
        return thread_.joinable();
    }

    void Join()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    void Detach()
    {
        if (thread_.joinable())
        {
            thread_.detach();
        }
    }

private:
    std::thread thread_;
};

} // namespace cigorn

#endif