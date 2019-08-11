#include "stdafx.h"
#include "FileSystem.h"

FileSystem gFiles;

bool FileSystem::Initialize()
{
    char buffer[MAX_PATH + 1];
    if (::GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "FileSystem::FileSystem(), GetModuleFileNameA Failed");
        return false;
    }

    mExecutablePath.assign(buffer);
    mWorkingDirectoryPath = cxx::get_parent_directory(mExecutablePath);

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
    return false;
}

bool FileSystem::OpenTextFile(const char* objectName, std::ifstream& instream)
{
    return false;
}

bool FileSystem::IsDirectoryExists(const char* objectName)
{
    return false;
}

bool FileSystem::IsFileExists(const char* objectName)
{
    return false;
}