#include "stdafx.h"
#include "FileSystem.h"

//////////////////////////////////////////////////////////////////////////

static const std::string GTA1MapFileExtension = ".CMP";

//////////////////////////////////////////////////////////////////////////

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
    mGameMapsList.clear();
}

bool FileSystem::OpenBinaryFile(const std::string& objectName, std::ifstream& instream)
{
    instream.close();

    if (cxx::is_absolute_path(objectName))
    {
        instream.open(objectName, std::ios::in | std::ios::binary);
        return instream.is_open();
    }

    std::string pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer = cxx::va("%s/%s", currPlace.c_str(), objectName.c_str());
        if (cxx::is_file_exists(pathBuffer))
        {
            instream.open(pathBuffer, std::ios::in | std::ios::binary);
        }
    }
    return instream.is_open();
}

bool FileSystem::OpenTextFile(const std::string& objectName, std::ifstream& instream)
{
    instream.close();

    if (cxx::is_absolute_path(objectName))
    {
        instream.open(objectName, std::ios::in);
        return instream.is_open();
    }

    std::string pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer = cxx::va("%s/%s", currPlace.c_str(), objectName.c_str());
        if (cxx::is_file_exists(pathBuffer))
        {
            instream.open(pathBuffer, std::ios::in);
        }
    }
    return instream.is_open();
}

bool FileSystem::IsDirectoryExists(const std::string& objectName)
{
    if (cxx::is_absolute_path(objectName))
    {
        return cxx::is_directory_exists(objectName);
    }
    std::string pathBuffer;
    // search directory in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer = cxx::va("%s/%s", currPlace.c_str(), objectName.c_str());
        if (cxx::is_directory_exists(pathBuffer))
            return true;
    }
    return false;
}

bool FileSystem::IsFileExists(const std::string& objectName)
{
    if (cxx::is_absolute_path(objectName))
        return cxx::is_file_exists(objectName);

    std::string pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer = cxx::va("%s/%s", currPlace.c_str(), objectName.c_str());
        if (cxx::is_file_exists(pathBuffer))
            return true;
    }
    return false;
}

bool FileSystem::ReadTextFile(const std::string& objectName, std::string& output)
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

void FileSystem::AddSearchPlace(const std::string& searchPlace)
{
    for (const std::string& currPlace: mSearchPlaces)
    {
        if (currPlace == searchPlace)
            return;
    }

    mSearchPlaces.emplace_back(searchPlace);
}

bool FileSystem::GetFullPathToFile(const std::string& objectName, std::string& fullPath) const
{
    if (cxx::is_absolute_path(objectName))
    {
        fullPath = objectName;
        return true;
    }
    std::string pathBuffer;
    // search directory in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer = cxx::va("%s/%s", currPlace.c_str(), objectName.c_str());
        if (cxx::is_file_exists(pathBuffer))
        {
            fullPath = pathBuffer;
            return true;
        }
    }
    return false;
}

bool FileSystem::SetupGtaDataLocation()
{
    const SystemStartupParams& startupParams = gSystem.mStartupParams;
    // override data location with startup param
    if (!startupParams.mGtaDataLocation.empty())
    {
        mGTADataDirectoryPath = startupParams.mGtaDataLocation;
    }

    if (mGTADataDirectoryPath.length())
    {
        if (!cxx::is_directory_exists(mGTADataDirectoryPath))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot locate gta gamedata: '%s'", mGTADataDirectoryPath.c_str());
            return false;
        }

        gFiles.AddSearchPlace(mGTADataDirectoryPath);
        gConsole.LogMessage(eLogMessage_Info, "Current gta gamedata location is: '%s'", mGTADataDirectoryPath.c_str());

        if (ScanGtaMaps())
        {
            gConsole.LogMessage(eLogMessage_Info, "Found gta maps:");
            for (const std::string& currMapname: mGameMapsList)
            {
                gConsole.LogMessage(eLogMessage_Info, " - %s", currMapname.c_str());
            }
        }
        else
        {
            gConsole.LogMessage(eLogMessage_Warning, "No gta maps found within search places");
            return false;
        }
        return true;
    }

    gConsole.LogMessage(eLogMessage_Error, "Location of gta gamedata is not specified");
    return false;
}

bool FileSystem::ScanGtaMaps()
{
    mGameMapsList.clear();

    for (const std::string& currSearchPlace: mSearchPlaces)
    {
        cxx::enum_files(currSearchPlace, [this](const std::string& curr)
        {
            if (cxx::get_file_extension(curr) == GTA1MapFileExtension)
            {
                mGameMapsList.push_back(curr);  
            }
        });
    }

    return !mGameMapsList.empty();
}
