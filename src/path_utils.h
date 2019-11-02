#pragma once

namespace cxx
{
    using enum_files_proc = std::function<void (const std::string&)>;

    // extract parent directory for element
    // @param sourcePath: Path
    std::string get_parent_directory(std::string pathto);

    // extract file name without extension
    // @param sourcePath: Path
    std::string get_name_without_extension(std::string pathto);

    // extract file name with extension
    // @param sourcePath: Path
    std::string get_file_name(std::string pathto);

    // extract file extension
    // @param sourcePath: Path
    std::string get_file_extension(std::string pathto);

    // get full path to executable
    std::string get_executable_path();

    // test whether path is absolute or relative
    // @param sourcePath: Path
    bool is_absolute_path(std::string pathto);

    // test whether file exists in os
    // @param sourcePath: Path
    bool is_file_exists(std::string pathto);

    // test whether directory exists in os
    // @param sourcePath: Path
    bool is_directory_exists(std::string pathto);

    // create directories in path
    bool ensure_path_exists(std::string pathto);

    // enumerate files and directories at specific location
    // @param pathto: Location
    // @param enumproc: Enumeration callback
    void enum_files(std::string pathto, enum_files_proc enumproc);
    void enum_files_recursive(std::string pathto, enum_files_proc enumproc);

} // namespace cxx