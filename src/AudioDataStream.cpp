#include "stdafx.h"
#include "AudioDataStream.h"
#include "wave_utils.h"
#include "FileSystem.h"

//////////////////////////////////////////////////////////////////////////

const char* AudioFileExtWAVE = ".wav";

//////////////////////////////////////////////////////////////////////////

// wave file stream implementation

class WaveFileStream: public AudioDataStream
{
public:
    WaveFileStream()
        : mWaveReader(mFileStream)
    {
    }
    bool OpenFileStream(const std::string& fileName)
    {
        if (!gFiles.OpenBinaryFile(fileName, mFileStream))
            return false;

        return mWaveReader.parse_audio();
    }

    int GetChannelsCount() const override { return mWaveReader.mChannelsCount; }
    int GetSampleRate() const override { return mWaveReader.mSampleRate; }
    int GetSampleBits() const override { return mWaveReader.mSampleBits; }
    int GetSamplesCount() const override { return mWaveReader.mSamplesCount; }

    int ReadPCMSamples(int samples, void* buffer) override
    {
        return mWaveReader.read_pcm_samples(samples, buffer);
    }
    bool SeekPCMFromBeg(int samples) override
    {
        return mWaveReader.seek_pcm_beg(samples);
    }
    bool SeekPCMFromEnd(int samples) override
    {
        return mWaveReader.seek_pcm_end(samples);
    }
    bool SeekPCMFromCur(int samples) override
    {
        return mWaveReader.seek_pcm_cur(samples);
    }
    bool EndOfStream() const override
    {
        return mWaveReader.end_of_stream();
    }

private:
    std::ifstream mFileStream;
    cxx::wave_reader mWaveReader;
};

//////////////////////////////////////////////////////////////////////////

extern AudioDataStream* OpenAudioFileStream(const char* fileName)
{
    std::string fileNameWithExt;
    std::string fileExt = cxx::get_file_extension(fileName);
    // detect file extension
    if (fileExt.empty())
    {
        static const char* knownAudioExtensions[] =
        {
            AudioFileExtWAVE
        };

        for (const char* currExt: knownAudioExtensions)
        {
            std::string currFileName = cxx::va("%s%s", fileName, currExt);
            if (gFiles.IsFileExists(currFileName))
            {
                fileExt = currExt;
                fileNameWithExt = currFileName;
                break;
            }
        }

        if (fileExt.empty())
            return nullptr;
    }
    else
    {
        fileNameWithExt = fileName;

        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
    }

    AudioDataStream* audioDataStream = nullptr;

    if (fileExt == AudioFileExtWAVE)
    {
        WaveFileStream* waveFileStream = new WaveFileStream;
        if (!waveFileStream->OpenFileStream(fileNameWithExt))
        {
            SafeDelete(waveFileStream);
        }
        audioDataStream = waveFileStream;
    }
    return audioDataStream;
}

