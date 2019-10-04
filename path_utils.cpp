#include "stdafx.h"
#include "path_utils.h"
#include <experimental/filesystem>

#if OS_NAME == OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#elif OS_NAME == OS_LINUX
    #include <limits.h>
    #include <unistd.h>
#endif

namespace filesystem = std::experimental::filesystem;
// details
namespace os = std::tr2::sys;

namespace cxx
{

std::string get_parent_directory(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return sourcePath.parent_path().generic_string();
}

std::string get_name_without_extension(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return sourcePath.stem().generic_string();
}

std::string get_file_name(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return sourcePath.filename().generic_string();
}

std::string get_file_extension(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return sourcePath.extension().generic_string();
}

std::string get_executable_path()
{
#if (OS_NAME == OS_WINDOWS)
    char buffer[MAX_PATH + 1];
    int count = ::GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer, (count > 0) ? count : 0);
#elif (OS_NAME == OS_LINUX)
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    return std::string(buffer, (count > 0) ? count : 0);
#else
    debug_assert(false);
    return std::string();
#endif
}

bool is_absolute_path(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return sourcePath.is_absolute();
}

bool is_file_exists(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return os::is_regular_file(pathto);
}

bool is_directory_exists(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return os::is_directory(pathto);
}

bool ensure_path_exists(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return filesystem::create_directories(sourcePath);
}

} // namespace cxx