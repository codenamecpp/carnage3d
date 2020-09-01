#include "stdafx.h"
#include "GameTextsManager.h"

GameTextsManager gGameTexts;

bool GameTextsManager::Initialize()
{
    mErrorString = "***Text missed***";
    return true;
}

void GameTextsManager::Deinit()
{
    mStrings.clear();
}

const std::string& GameTextsManager::GetText(const std::string& textID) const
{
    auto find_iterator = mStrings.find(textID);
    if (find_iterator == mStrings.end())
        return mErrorString;

    return find_iterator->second;
}

bool GameTextsManager::LoadTexts(const std::string& fileName)
{
    std::vector<unsigned char> fileContent;
    if (!gFiles.ReadBinaryFile(fileName, fileContent))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load game texts from file '%s'", fileName.c_str());
        return false;
    }

    unsigned char enc;
    char offset;
    if (fileContent[0] == 0xBF) 
    {
        enc = 0x63;
        offset = -1;
    } 
    else if (fileContent[0] == 0xA6) 
    {
        enc = 0x67;
        offset = 28;
    } 
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "Invalid texts file format '%s'", fileName.c_str());
        return false;
    }

    for (int i = 0; i < 8; i++) 
    {
        fileContent[i] = fileContent[i] - enc;
        enc <<= 1;
    }

    for (unsigned char& currData: fileContent) 
    {
        currData += offset;
    }

    mStrings.clear();

    // parse lines
    std::string keyString;
    for (auto cursor_iterator = fileContent.begin();;)
    {
        auto key_iterator_a = std::find(cursor_iterator, fileContent.end(), '[');
        auto key_iterator_b = std::find(key_iterator_a, fileContent.end(), ']');
        if (key_iterator_a == fileContent.end() ||
            key_iterator_b == fileContent.end())
        {
            break;
        }

        auto value_iterator_a = key_iterator_b;
        auto value_iterator_b = std::find(value_iterator_a, fileContent.end(), 0);
        if (value_iterator_a == fileContent.end() ||
            value_iterator_b == fileContent.end())
        {
            break;
        }

        keyString.assign(++key_iterator_a, key_iterator_b);
        if (keyString.empty())
            break;

        mStrings[keyString].assign(++value_iterator_a, value_iterator_b);
        cursor_iterator = ++value_iterator_b;
    }
    return true;
}
