#include "stdafx.h"
#include "FileSystem.h"

FileSystem gFiles;

bool FileSystem::Initialize()
{
    mExecutablePath = cxx::get_executable_path();
    mWorkingDirectoryPath = cxx::get_parent_directory(mExecutablePath);
    gConsole.LogMessage(eLogMessage_Info, "Working directory: '%s'", mWorkingDirectoryPath.c_str());

//#ifdef _DEBUG
    std::string debugDataPath = cxx::get_parent_directory(mWorkingDirectoryPath);
    mSearchPlaces.emplace_back(debugDataPath + "/gamedata");
//#else
    mSearchPlaces.emplace_back(mWorkingDirectoryPath + "/gamedata");
//#endif
    return true;
}

void FileSystem::Deinit()
{
    mExecutablePath.clear();
    mWorkingDirectoryPath.clear();
    mGTADataDirectoryPath.clear();
}

bool FileSystem::OpenBinaryFile(const char* objectName, std::ifstream& instream)
{
    instream.close();

    if (cxx::is_absolute_path(objectName))
    {
        instream.open(objectName, std::ios::in | std::ios::binary);
        return instream.is_open();
    }

    cxx::string_buffer_512 pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_file_exists(pathBuffer.c_str()))
        {
            instream.open(pathBuffer.c_str(), std::ios::in | std::ios::binary);
        }
    }
    return instream.is_open();
}

bool FileSystem::OpenTextFile(const char* objectName, std::ifstream& instream)
{
    instream.close();

    if (cxx::is_absolute_path(objectName))
    {
        instream.open(objectName, std::ios::in);
        return instream.is_open();
    }

    cxx::string_buffer_512 pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_file_exists(pathBuffer.c_str()))
        {
            instream.open(pathBuffer.c_str(), std::ios::in);
        }
    }
    return instream.is_open();
}

bool FileSystem::IsDirectoryExists(const char* objectName)
{
    if (cxx::is_absolute_path(objectName))
    {
        return cxx::is_directory_exists(objectName);
    }
    cxx::string_buffer_512 pathBuffer;
    // search directory in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_directory_exists(pathBuffer.c_str()))
            return true;
    }
    return false;
}

bool FileSystem::IsFileExists(const char* objectName)
{
    if (cxx::is_absolute_path(objectName))
        return cxx::is_file_exists(objectName);

    cxx::string_buffer_512 pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_file_exists(pathBuffer.c_str()))
            return true;
    }
    return false;
}

bool FileSystem::ReadTextFile(const char* objectName, std::string& output)
{
    output.clear();

    std::ifstream fileStream;
    if (!OpenTextFile(objectName, fileStream))
        return false;

    std::string stringLine {};
    while (std::getline(fileStream, stringLine, '\n'))
    {
        output.append(stringLine);
        output.append("\n");
    }
    return true;
}

void FileSystem::AddSearchPlace(const char* searchPlace)
{
    for (const std::string& currPlace: mSearchPlaces)
    {
        if (currPlace == searchPlace)
            return;
    }

    mSearchPlaces.emplace_back(searchPlace);
}

bool FileSystem::GetFullPathToFile(const char* objectName, std::string& fullPath) const
{
    if (cxx::is_absolute_path(objectName))
    {
        fullPath = objectName;
        return true;
    }
    cxx::string_buffer_512 pathBuffer;
    // search directory in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_file_exists(pathBuffer.c_str()))
        {
            fullPath = pathBuffer.c_str();
            return true;
        }
    }
    return false;
}

bool FileSystem::SetupGtaDataLocation()
{
    const SysStartupParameters& startupParams = gSystem.mStartupParams;
    // override data location with startup param
    if (startupParams.mGtaDataLocation.get_length())
    {
        mGTADataDirectoryPath = startupParams.mGtaDataLocation.c_str();
    }

    if (mGTADataDirectoryPath.length())
    {
        if (!cxx::is_directory_exists(mGTADataDirectoryPath))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot locate gta gamedata: '%s'", mGTADataDirectoryPath.c_str());
            return false;
        }

        gFiles.AddSearchPlace(mGTADataDirectoryPath.c_str());
        gConsole.LogMessage(eLogMessage_Info, "Current gta gamedata location is: '%s'", mGTADataDirectoryPath.c_str());
        return true;
    }

    gConsole.LogMessage(eLogMessage_Error, "Location of gta gamedata is not specified");
    return false;
}
