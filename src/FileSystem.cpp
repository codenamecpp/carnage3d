#include "stdafx.h"
#include "FileSystem.h"
#include "cvars.h"

//////////////////////////////////////////////////////////////////////////

static const char* GTA1MapFileExtension = ".CMP";

//////////////////////////////////////////////////////////////////////////

// cvars
CvarString gCvarGtaDataPath("g_gtadata", "", "GTA data location", CvarFlags_Archive | CvarFlags_Init | CvarFlags_Hidden);

//////////////////////////////////////////////////////////////////////////

FileSystem gFiles;

bool FileSystem::Initialize()
{
    mExecutablePath = cxx::get_executable_path();
    mWorkingDirectoryPath = cxx::get_parent_directory(mExecutablePath);
    mWorkingDirectoryPath = cxx::get_parent_directory(mWorkingDirectoryPath); // root
    if (!mWorkingDirectoryPath.empty())
    {
        mWorkingDirectoryPath.append("/");
    }
    mWorkingDirectoryPath.append("gamedata");
    AddSearchPlace(mWorkingDirectoryPath);

    gConsole.LogMessage(eLogMessage_Info, "Working directory: '%s'", mWorkingDirectoryPath.c_str());
    return true;
}

void FileSystem::Deinit()
{
    mExecutablePath.clear();
    mWorkingDirectoryPath.clear();
    mGameMapsList.clear();
}

bool FileSystem::OpenBinaryFile(const std::string& objectName, std::ifstream& instream)
{
    instream.close();

    if (cxx::is_file_exists(objectName))
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

    if (cxx::is_file_exists(objectName))
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

bool FileSystem::CreateBinaryFile(const std::string& objectName, std::ofstream& outstream)
{
    outstream.close();

    std::string path;
    if (!mWorkingDirectoryPath.empty())
    {
        path = cxx::va("%s/%s", mWorkingDirectoryPath.c_str(), objectName.c_str());
    }
    else
    {
        path = objectName;
    }
    outstream.open(path, std::ios::out | std::ios::binary);
    return outstream.is_open();
}

bool FileSystem::CreateTextFile(const std::string& objectName, std::ofstream& outstream)
{
    outstream.close();

    std::string path;
    if (!mWorkingDirectoryPath.empty())
    {
        path = cxx::va("%s/%s", mWorkingDirectoryPath.c_str(), objectName.c_str());
    }
    else
    {
        path = objectName;
    }
    outstream.open(path, std::ios::out);
    return outstream.is_open();
}

bool FileSystem::IsDirectoryExists(const std::string& objectName)
{
    if (cxx::is_directory_exists(objectName))
        return true;

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
    if (cxx::is_file_exists(objectName))
        return true;

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

bool FileSystem::ReadBinaryFile(const std::string& objectName, std::vector<unsigned char>& output)
{
    output.clear();

    std::ifstream fileStream;
    if (!OpenBinaryFile(objectName, fileStream))
        return false;

    fileStream.seekg(0, std::ios::end);
    size_t fileSize = (size_t) fileStream.tellg();
    fileStream.seekg(0);

    output.resize(fileSize);
    if (!fileStream.read((char*) output.data(), fileSize))
    {
        output.clear();
        return false;
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
    if (cxx::is_file_exists(objectName))
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

bool FileSystem::GetFullPathToDirectory(const std::string& objectName, std::string& fullPath) const
{
    if (cxx::is_directory_exists(objectName))
    {
        fullPath = objectName;
        return true;
    }
    std::string pathBuffer;
    // search directory in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer = cxx::va("%s/%s", currPlace.c_str(), objectName.c_str());
        if (cxx::is_directory_exists(pathBuffer))
        {
            fullPath = pathBuffer;
            return true;
        }
    }
    return false;
}

bool FileSystem::SetupGtaDataLocation()
{
    // override data location with startup param
    if (gCvarCurrentBaseDir.mValue.empty())
    {
        gCvarCurrentBaseDir.mValue = gCvarGtaDataPath.mValue;
    }

    if (gCvarCurrentBaseDir.mValue.length())
    {
        if (!IsDirectoryExists(gCvarCurrentBaseDir.mValue))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot locate gta gamedata: '%s'", gCvarCurrentBaseDir.mValue.c_str());
            return false;
        }

        gConsole.LogMessage(eLogMessage_Info, "Current gta gamedata location is: '%s'", gCvarCurrentBaseDir.mValue.c_str());

        GetFullPathToDirectory(gCvarCurrentBaseDir.mValue, gCvarCurrentBaseDir.mValue);
        gFiles.AddSearchPlace(gCvarCurrentBaseDir.mValue);

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

bool FileSystem::ReadConfig(const std::string& filePath, cxx::json_document& configDocument)
{
    std::string configContent;
    if (!ReadTextFile(filePath, configContent))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open config file '%s'", filePath.c_str());
        return false;
    }

    if (!configDocument.parse_document(configContent))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse config file '%s'", filePath.c_str());
        return false;
    }
    return true;
}

bool FileSystem::SaveConfig(const std::string& filePath, const cxx::json_document& configDocument)
{
    std::ofstream outputFile;
    if (!CreateTextFile(filePath, outputFile))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot write config file '%s'", filePath.c_str());
        return false;
    }

    std::string documentContent;
    configDocument.dump_document(documentContent);

    outputFile << documentContent;
    return true;
}