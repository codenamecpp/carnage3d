#include "stdafx.h"
#include "GameTextsManager.h"

GameTextsManager gGameTexts;

//////////////////////////////////////////////////////////////////////////

class FXTReader
{
public:
/*
    https://gta.mendelsohn.de/Reference/Fil_FXT.html

    Decryption
    The messages are encrypted using a polyalphabetic substitution cipher that appears like a monoalphabetic cypher after byte 8, which made it easy to crack.

    Subtract an offset from the first 8 bytes in the file. The respective offsets are, in hexadecimal notation, 
        63, c6, 8c, 18, 30, 60, c0, 80. (Of course you can get these numbers by taking 63 and shifting it one bit to left for each consecutive number.
        You can see why this sequence ends after 8 bytes; it would‘ve been harder to crack, had the bit pattern been rotated instead of shifted...)

    Subtract 1 from all bytes. (This includes the bytes from the previous step!)

    If a byte equals 195, delete it and add 64 to the next byte. This relocates extended characters to the windows ANSI character set, 
    which is necessary for three of the four languages provided.

    Each string ends with a 0 byte and contains no CR or LF codes. The next string is stored immediately after it. 
    The last string in the file seems to always be "[]“.
*/

    FXTReader(std::istream& inputStream)
        : mInputStream(inputStream)
        , mBytesCounter()
    {
    }

    bool get_next_key_value(std::string& outputKey, std::string& outputValue)
    {
        if (!get_next_key(outputKey) || outputKey.empty())
            return false;

        if (!get_next_value(outputValue))
            return false;

        return true;
    }

private:
    bool get_next_key(std::string& outputKey)
    {
        unsigned char currChar;
        if (!get_next_char(currChar))
            return false;

        if (currChar != '[')
            return false;

        outputKey.clear();
        for (;;)
        {
            if (!get_next_char(currChar))
                return false;

            if (currChar == ']')
                break;

            outputKey.push_back(currChar);
        }

        return true;
    }

    bool get_next_value(std::string& outputValue)
    {
        outputValue.clear();
        for (unsigned char currChar;;)
        {
            if (!get_next_char(currChar))
                return false;

            if (currChar == 0)
                break;

            outputValue.push_back(currChar);
        }
        return true;
    }

    bool get_next_char(unsigned char& character)
    {
        const static unsigned char code[] = {0x64, 0xc7, 0x8d, 0x19, 0x31, 0x61, 0xc1, 0x81};
        if (mBytesCounter < CountOf(code))
        {
            if (mInputStream.read((char*) &character, 1))
            {
                character = character - code[mBytesCounter++];
                return true;
            }
            return false;
        }

        if (!mInputStream.read((char*) &character, 1))
            return false;

        ++mBytesCounter;
		if (--character == 195) 
        {
            if (!mInputStream.read((char*) &character, 1))
            {
                debug_assert(false);
                return false;
            }
            ++mBytesCounter;
            character = --character + 64;
		};
        return true;
    }

private:
    std::istream& mInputStream;
    int mBytesCounter;
};

//////////////////////////////////////////////////////////////////////////

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
    std::ifstream fileStream;
    if (!gFiles.OpenBinaryFile(fileName, fileStream))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open texts file '%s'", fileName.c_str());
        return false;
    }

    mStrings.clear();

    FXTReader fxtreader(fileStream);
    
    std::string key;
    std::string value;

    for (;;)
    {
        if (!fxtreader.get_next_key_value(key, value))
            break;

        mStrings[key] = value;
    }
    
    return true;
}
