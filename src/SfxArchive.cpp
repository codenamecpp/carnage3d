#include "stdafx.h"
#include "SfxArchive.h"

SfxArchive::~SfxArchive()
{
    FreeArchive();
}

bool SfxArchive::LoadArchive(const std::string& archiveName)
{
    FreeArchive();

    std::string metaName = cxx::va("%s.SDT", archiveName.c_str());
    std::string dataName = cxx::va("%s.RAW", archiveName.c_str());
    // read meta information
    
    std::ifstream metaFile;
    if (!gFiles.OpenBinaryFile(metaName, metaFile))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open audio metadata '%s'", metaName.c_str());
        return false;
    }

    if (!gFiles.OpenBinaryFile(dataName, mRawDataStream))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open audio data '%s'", dataName.c_str());
        return false;
    }

    struct sdt_entry_info
    {
        unsigned int mDataOffset;
        unsigned int mDataLength;
        unsigned int mSampleRate;
    };

    const int Sizeof_SDT_EntryInfo = sizeof(sdt_entry_info);
    
    // get num entries
    metaFile.seekg(0, std::ios::end);
    unsigned int fileSize = (unsigned int) metaFile.tellg();
    metaFile.seekg(0);

    unsigned int entriesCount = (fileSize / Sizeof_SDT_EntryInfo);
    debug_assert((fileSize % Sizeof_SDT_EntryInfo) == 0);
    if (entriesCount == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Could not find any audio entries in '%s'", archiveName.c_str());

        FreeArchive();
        return false;
    }

    mAudioEntries.resize(entriesCount);

    bool mainMenuSounds = cxx::has_suffix(archiveName.c_str(), "000"); // hack

    // get entries
    for (unsigned int icurr = 0; icurr < entriesCount; ++icurr)
    {
        sdt_entry_info currEntrySrc;
        if (!metaFile.read((char*) &currEntrySrc, Sizeof_SDT_EntryInfo))
        {
            debug_assert(false);
            continue;
        }
        
        SfxArchiveEntry& currEntry = mAudioEntries[icurr];
        currEntry.mDataOffset = currEntrySrc.mDataOffset;
        currEntry.mDataLength = currEntrySrc.mDataLength;
        currEntry.mSampleRate = currEntrySrc.mSampleRate;
        currEntry.mBitsPerSample = mainMenuSounds ? 16 : 8;
        currEntry.mChannelsCount = (mainMenuSounds && icurr < 3) ? 2 : 1;
    }

    return true;
}

void SfxArchive::FreeArchive()
{
    for (SfxArchiveEntry& currEntry: mAudioEntries)
    {
        SafeDeleteArray(currEntry.mData);
    }
    mAudioEntries.clear();
    mRawDataStream.close();
}

bool SfxArchive::IsLoaded() const
{
    return !mAudioEntries.empty();
}

int SfxArchive::GetEntriesCount() const
{
    return (int) mAudioEntries.size();
}

bool SfxArchive::GetEntryInfo(int entryIndex, SfxArchiveEntry& output) const
{
    int MaxEntriesCount = GetEntriesCount();
    if (entryIndex < MaxEntriesCount)
    {
        output = mAudioEntries[entryIndex];
        return true;
    }
    debug_assert(false);
    return false;
}

bool SfxArchive::GetEntryData(int entryIndex, SfxArchiveEntry& output)
{
    int MaxEntriesCount = GetEntriesCount();
    if (entryIndex < MaxEntriesCount)
    {
        SfxArchiveEntry& currEntry = mAudioEntries[entryIndex];
        if (currEntry.mData == nullptr) // force load audio data from raw stream
        {
            currEntry.mData = new unsigned char[currEntry.mDataLength];
            mRawDataStream.seekg(currEntry.mDataOffset);
            mRawDataStream.read((char*)currEntry.mData, currEntry.mDataLength);
        }

        output = currEntry;
        return true;
    }
    debug_assert(false);
    return false;
}

void SfxArchive::FreeEntryData(int entryIndex)
{
    int MaxEntriesCount = GetEntriesCount();
    if (entryIndex < MaxEntriesCount)
    {
        SfxArchiveEntry& currEntry = mAudioEntries[entryIndex];
        SafeDeleteArray(currEntry.mData);
    }
    else
    {
        debug_assert(false);
    }
}

void SfxArchive::DumpSounds(const std::string& outputDirectory)
{
    cxx::ensure_path_exists(outputDirectory);
    
    struct wave_header
    {
        char mChunkID[4] = {'R','I','F','F'};
        int mChunkSize = 49188;   // size not including chunkSize or chunkID
        char mFormat[4] = {'W','A','V','E'};
        char mSubChunkID[4] = {'f','m','t',' '};
        int mSubChunkSize = 16;
        short mAudioFormat = 1;
        short mNumChannels = 1;
        int mSampleRate = 8;
        int mByteRate = 8;
        short mBlockAlign = 1;
        short mBitsPerSample = 8;
        char mSubChunk2ID[4] = {'d','a','t','a'};
        int mSubChunk2Size; // Stores the size of the data block
    };
    const int Sizeof_WaveHeader = sizeof(wave_header);

    wave_header waveHeader;

    int numEntries = GetEntriesCount();
    for (int icurr = 0; icurr < numEntries; ++icurr)
    {
        SfxArchiveEntry audioArchiveEntry;
        if (!GetEntryData(icurr, audioArchiveEntry))
        {
            debug_assert(false);
            continue;
        }
        waveHeader.mChunkSize = audioArchiveEntry.mDataLength + Sizeof_WaveHeader;
        waveHeader.mSampleRate = audioArchiveEntry.mSampleRate;
        waveHeader.mByteRate = (audioArchiveEntry.mSampleRate * audioArchiveEntry.mBitsPerSample * audioArchiveEntry.mChannelsCount) / 8;
        waveHeader.mBitsPerSample = audioArchiveEntry.mBitsPerSample;
        waveHeader.mNumChannels = audioArchiveEntry.mChannelsCount;
        waveHeader.mSubChunk2Size = audioArchiveEntry.mDataLength;

        std::string filePath = cxx::va("%s/%d.wav", outputDirectory.c_str(), icurr);
        std::ofstream outputFile (filePath, std::ios::binary);
        outputFile.write((const char*)&waveHeader, Sizeof_WaveHeader);
        outputFile.write((const char*)audioArchiveEntry.mData, audioArchiveEntry.mDataLength);
    }
}
