#pragma once

#include <filesystem>

namespace cxx
{
    namespace filesystem = std::experimental::filesystem;

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

} // namespace cxx