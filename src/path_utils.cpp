#include "stdafx.h"
#include "path_utils.h"

#if OS_NAME == OS_MACOS
    namespace filesystem = std::filesystem;
#elif defined(__EMSCRIPTEN__)
#include <filesystem>
    namespace filesystem = std::__fs::filesystem;
#else
#include <experimental/filesystem>
    namespace filesystem = std::experimental::filesystem;
#endif // #ifndef __EMSCRIPTEN__

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
#elif (OS_NAME == OS_MACOS)
    char buffer[PATH_MAX];
    uint32_t count = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &count) == 0) {
        char resolved[PATH_MAX];
        realpath(buffer, resolved);

        return std::string(resolved, count);
    }
#else
    debug_assert(false);
    return std::string();
#endif
}

bool is_file_exists(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return filesystem::is_regular_file(pathto);
}

bool is_directory_exists(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return filesystem::is_directory(pathto);
}

bool ensure_path_exists(std::string pathto)
{
    filesystem::path sourcePath {pathto};
    return filesystem::create_directories(sourcePath);
}

void enum_files(std::string pathto, enum_files_proc enumproc)
{
    filesystem::path sourcePath {pathto};
    if (!filesystem::exists(sourcePath))
        return;

    filesystem::directory_iterator iter_directory_end;
    for (filesystem::directory_iterator iter_directory(sourcePath);
        iter_directory != iter_directory_end; ++iter_directory)
    {
        const filesystem::path& currentFile = iter_directory->path();
        enumproc(currentFile.filename().generic_string());
    }
}

void enum_files_recursive(std::string pathto, enum_files_proc enumproc)
{
    filesystem::path sourcePath {pathto};
    if (!filesystem::exists(sourcePath))
        return;

    filesystem::recursive_directory_iterator iter_directory_end;
    for (filesystem::recursive_directory_iterator iter_directory(sourcePath);
        iter_directory != iter_directory_end; ++iter_directory)
    {
        const filesystem::path& currentFile = iter_directory->path();
        enumproc(currentFile.filename().generic_string());
    }
}

} // namespace cxx
