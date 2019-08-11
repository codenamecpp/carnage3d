#pragma once

#include <filesystem>

namespace cxx
{
    namespace filesystem = std::experimental::filesystem;

    // details
    namespace os = std::tr2::sys;

    // extract parent directory for element
    // @param sourcePath: Path
    inline std::string get_parent_directory(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return sourcePath.parent_path().generic_string();
    }

    // extract file name without extension
    // @param sourcePath: Path
    inline std::string get_name_without_extension(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return sourcePath.stem().generic_string();
    }

    // extract file name with extension
    // @param sourcePath: Path
    inline std::string get_file_name(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return sourcePath.filename().generic_string();
    }

    // extract file extension
    // @param sourcePath: Path
    inline std::string get_file_extension(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return sourcePath.extension().generic_string();
    }

    // test whether path is absolute or relative
    // @param sourcePath: Path
    inline bool is_absolute_path(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return sourcePath.is_absolute();
    }

    // helper function
    inline std::string join_paths(std::string pathto)
    {
        return std::move(pathto);
    }

    // combine path components
    // @param pathto: First path component
    // @param args: Path components
    template<typename ...Args>
    inline std::string join_paths(std::string pathto, Args && ... args)
    {
        filesystem::path headPath {pathto};
        filesystem::path tailPath = join_paths(std::forward<Args>(args)...);

        headPath += tailPath;
        return headPath.generic_string();
    }

    // test whether file exists in os
    // @param sourcePath: Path
    inline bool is_file_exists(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return os::is_regular_file(pathto);
    }

    // test whether directory exists in os
    // @param sourcePath: Path
    inline bool is_directory_exists(std::string pathto)
    {
        filesystem::path sourcePath {pathto};
        return os::is_directory(pathto);
    }

} // namespace cxx