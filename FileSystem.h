#pragma once

// file system manager
class FileSystem final: public cxx::noncopyable
{
public:
    // standard places paths
    std::string mExecutablePath;
    std::string mWorkingDirectoryPath;
    std::string mGTADataDirectoryPath;
    std::vector<std::string> mSearchPlaces;

public:
    // Setup filesystem internal resources
    bool Initialize();

    // Free allocated resources
    void Deinit();

    // Add search place directory
    // @param searchPlace: Path
    void AddSearchPlace(const char* searchPlace);

    // Open text or binary file stream for reading operations
    // @param objectName: File name
    // @param instream: Output stream
    bool OpenBinaryFile(const char* objectName, std::ifstream& instream);
    bool OpenTextFile(const char* objectName, std::ifstream& instream);

    // Load whole text file content to std string
    // @param objectName: File name
    // @param output: Content
    bool ReadTextFile(const char* objectName, std::string& output);

    // Test whether file exists
    // @param objectName: File name
    bool IsFileExists(const char* objectName);

    // Test whether directory exists
    // @param objectName: Directory name
    bool IsDirectoryExists(const char* objectName);
};

extern FileSystem gFiles;