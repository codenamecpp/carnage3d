#pragma once

// Audio entry information within archive
struct SfxArchiveEntry
{
    unsigned int mDataOffset = 0;
    unsigned int mDataLength = 0;
    unsigned int mSampleRate = 0;
    unsigned int mBitsPerSample = 0;
    unsigned int mChannelsCount = 0;

    unsigned char* mData = nullptr;
};

// Contains audio entries within archive
class SfxArchive final: public cxx::noncopyable
{
public:
    SfxArchive() = default;
    ~SfxArchive();

    // Load audio entries from archive
    // @param archiveName: Achive name without extension
    bool LoadArchive(const std::string& archiveName);
    void FreeArchive();
    bool IsLoaded() const;

    // Reading audio entries
    bool GetEntryInfo(int entryIndex, SfxArchiveEntry& output) const;
    bool GetEntryData(int entryIndex, SfxArchiveEntry& output);
    int GetEntriesCount() const;

    // Unload entry data from memory
    void FreeEntryData(int entryIndex);

    // Save all audio entries to wav files
    void DumpSounds(const std::string& outputDirectory);

private:
    std::vector<SfxArchiveEntry> mAudioEntries;
    std::ifstream mRawDataStream;
};