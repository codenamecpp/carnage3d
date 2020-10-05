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

    std::vector<std::string> mGameMapsList;

public:
    // Setup filesystem internal resources
    bool Initialize();

    // Free allocated resources
    void Deinit();

    // Init gta gamedata files location
    bool SetupGtaDataLocation();
    
    // Add search place directory
    // @param searchPlace: Path
    void AddSearchPlace(const std::string& searchPlace);

    // Open text or binary file stream for reading operations
    // @param objectName: File name
    // @param instream: Output stream
    bool OpenBinaryFile(const std::string& objectName, std::ifstream& instream);
    bool OpenTextFile(const std::string& objectName, std::ifstream& instream);

    // Load whole text file content to std string
    // @param objectName: File name
    // @param output: Content
    bool ReadTextFile(const std::string& objectName, std::string& output);

    // Load whole binary file content to std vector
    bool ReadBinaryFile(const std::string& objectName, std::vector<unsigned char>& output);
    
    // Load json config document
    bool ReadConfig(const std::string& jsonName, cxx::json_document& output);

    // Test whether file exists
    // @param objectName: File name
    bool IsFileExists(const std::string& objectName);

    // Test whether directory exists
    // @param objectName: Directory name
    bool IsDirectoryExists(const std::string& objectName);

    // Find file within search places and get full path to it
    // @param objectName: File name
    // @param fullPath: Out full path
    bool GetFullPathToFile(const std::string& objectName, std::string& fullPath) const;
    bool GetFullPathToDirectory(const std::string& objectName, std::string& fullPath) const;

private:
    // Gather all gta maps within gamedata
    bool ScanGtaMaps();
};

extern FileSystem gFiles;