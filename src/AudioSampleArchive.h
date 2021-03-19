#pragma once

#include "SfxDefs.h"

// Contains audio entries within SDT/RAW archive
class AudioSampleArchive final: public cxx::noncopyable
{
public:

    // Audio entry information within archive
    struct SampleEntry
    {
        unsigned int mDataOffset = 0;
        unsigned int mDataLength = 0;
        unsigned int mSampleRate = 0;
        unsigned int mBitsPerSample = 0;
        unsigned int mChannelsCount = 0;
        unsigned char* mData = nullptr;
    };

public:
    AudioSampleArchive() = default;
    ~AudioSampleArchive();

    // Load audio entries from archive
    // @param archiveName: Achive name without extension
    bool LoadArchive(const std::string& archiveName);
    void FreeArchive();
    bool IsLoaded() const;

    // Reading audio entries
    bool GetEntryInfo(int entryIndex, SampleEntry& output) const;
    bool GetEntryData(int entryIndex, SampleEntry& output);
    int GetEntriesCount() const;

    // Unload entry data from memory
    void FreeEntryData(int entryIndex);

    // Save all audio entries to wav files
    void DumpSounds(const std::string& outputDirectory);

private:
    std::vector<SampleEntry> mAudioEntries;
    std::ifstream mRawDataStream;
};