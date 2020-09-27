#pragma once

// Audio entry information within archive
struct AudioArchiveEntry
{
    unsigned int mDataOffset = 0;
    unsigned int mDataLength = 0;
    unsigned int mSampleRate = 0;
    unsigned int mBitsPerSample = 0;
    unsigned int mChannelsCount = 0;

    unsigned char* mData = nullptr;
};

// Contains audio entries within archive
class AudioArchive final: public cxx::noncopyable
{
public:
    AudioArchive() = default;
    ~AudioArchive();

    // Load audio entries from archive
    // @param archiveName: Achive name without extension
    bool LoadArchive(const std::string& archiveName);
    void FreeArchive();
    bool IsLoaded() const;

    // Reading audio entries
    bool GetEntryInfo(int entryIndex, AudioArchiveEntry& output) const;
    bool GetEntryData(int entryIndex, AudioArchiveEntry& output);
    int GetEntriesCount() const;

    // Unload entry data from memory
    void FreeEntryData(int entryIndex);

    // Save all audio entries to wav files
    void DumpSounds(const std::string& outputDirectory);

private:
    std::vector<AudioArchiveEntry> mAudioEntries;
    std::ifstream mRawDataStream;
};