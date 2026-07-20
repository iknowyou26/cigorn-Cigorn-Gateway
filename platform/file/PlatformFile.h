#ifndef CIGORN_PLATFORM_FILE_H
#define CIGORN_PLATFORM_FILE_H

#include <filesystem>
#include <string>

namespace cigorn
{
inline bool FileExists(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

inline bool DirectoryExists(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

inline std::string CurrentWorkingDirectory()
{
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    return ec ? std::string() : path.string();
}
}

#endif
