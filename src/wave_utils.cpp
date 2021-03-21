#include "stdafx.h"
#include "wave_utils.h"
#include "iostream_utils.h"

namespace cxx
{

// known wave chunk id's

const unsigned int WaveChunkID_RiffHeader = 0x46464952;
const unsigned int WaveChunkID_Format = 0x020746d66;
const unsigned int WaveChunkID_Data = 0x61746164;

// known riff format id's

const unsigned int RiffFormat_WAVE = 0x45564157;

// known wave format id's

const unsigned int WaveFormat_PCM = 1; // Pulse Code Modulation

// chunk header data
struct wave_chunk_header
{
    unsigned int mID; // one of WaveChunkID_*
    unsigned int mLength; // chunk data size in bytes, not incude header size
};

struct wave_pcm_format
{
    short mNumChannels; // 1 - mono, 2 - stereo
    int mSampleRate; 
    int mByteRate; // SampleRate * NumChannels * BitsPerSample/8
    short mBlockAlign; // NumChannels * BitsPerSample/8
    short mBitsPerSample;  
};

//////////////////////////////////////////////////////////////////////////

wave_reader::wave_reader(std::istream& inputStream)
    : mInputStream(inputStream)
    , mAudioDataStart()
{
}

bool wave_reader::parse_audio()
{
    if (!mInputStream)
        return false;

    clear();

    mInputStream.clear(); // force clear state after eof
    mInputStream.seekg(0, std::ios::beg);

    bool hasRiffHeaderChunk = false;
    bool hasFormatChunk = false;
    bool hasDataChunk = false;

    wave_pcm_format pcmFormat;

    for (;;)
    {
        wave_chunk_header chunkHeader;
        if (!read_from_stream(mInputStream, chunkHeader))
            break;

        if (chunkHeader.mID == WaveChunkID_RiffHeader)
        {
            hasRiffHeaderChunk = true;

            unsigned int riffFormat = 0;
            if (!read_from_stream(mInputStream, riffFormat))
            {
                debug_assert(false);
                return false;
            }

            if (riffFormat != RiffFormat_WAVE)
                return false;

            continue;
        }

        if (chunkHeader.mID == WaveChunkID_Format)
        {
            hasFormatChunk = true;

            short waveFormat = 0;
            if (!read_from_stream(mInputStream, waveFormat))
            {
                debug_assert(false);
                return false;
            }

            if (waveFormat != WaveFormat_PCM)
                return false;

            if (!read_from_stream(mInputStream, pcmFormat.mNumChannels) ||
                !read_from_stream(mInputStream, pcmFormat.mSampleRate) ||
                !read_from_stream(mInputStream, pcmFormat.mByteRate) ||
                !read_from_stream(mInputStream, pcmFormat.mBlockAlign) ||
                !read_from_stream(mInputStream, pcmFormat.mBitsPerSample))
            {
                return false;
            }
            continue;
        }

        if (chunkHeader.mID == WaveChunkID_Data)
        {
            hasDataChunk = true;
            mAudioDataLength = chunkHeader.mLength;
            mAudioDataStart = mInputStream.tellg();
            // skip audio data
            if (!mInputStream.seekg(chunkHeader.mLength, std::ios::cur))
            {
                debug_assert(false);
                return false;
            }
            continue;
        }

        // skip unknown chunk
        if (!mInputStream.seekg(chunkHeader.mLength, std::ios::cur))
        {
            debug_assert(false);
            break;
        }
    }

    if (!hasRiffHeaderChunk || !hasFormatChunk || !hasDataChunk)
        return false;

    // seek to audio data
    mInputStream.clear(); // force clear state after eof
    if (!mInputStream.seekg(mAudioDataStart, std::ios::beg))
    {
        debug_assert(false);
        return false;
    }

    mChannelsCount = pcmFormat.mNumChannels;
    mSampleRate = pcmFormat.mSampleRate;
    mSampleBits = pcmFormat.mBitsPerSample;
    mSamplesCount = mAudioDataLength / (mChannelsCount * mSampleBits / 8);
    mAudioDataEnd = mAudioDataStart + (std::streamoff) mAudioDataLength;

    return audio_present();
}

int wave_reader::read_pcm_samples(int samples, void* buffer)
{
    if (audio_present())
    {
        std::streampos currStreamPos = mInputStream.tellg();
        std::streamoff currOffset = (currStreamPos - mAudioDataStart);

        int currOffsetSamples = (int) currOffset / (mChannelsCount * (mSampleBits / 8));
        int canReadSamples = std::min(mSamplesCount - currOffsetSamples, samples);
        if (canReadSamples > 0)
        {
            int dataLength = (mChannelsCount * (mSampleBits / 8)) * canReadSamples;
            if (mInputStream.read((char*) buffer, dataLength))
                return canReadSamples;

            debug_assert(false);
        }
    }
    return 0;
}

bool wave_reader::seek_pcm_beg(int samples)
{
    if (audio_present())
    {
        std::streamoff dataOffset = (mChannelsCount * (mSampleBits / 8)) * samples;
        if (mInputStream.eof())
        {
            mInputStream.clear(); // force clear state after eof
        }
        if (mInputStream.seekg(mAudioDataStart + dataOffset, std::ios::beg))
            return true;
    }
    return false;
}

bool wave_reader::seek_pcm_end(int samples)
{
    if (audio_present())
    {
        std::streamoff dataOffset = (mChannelsCount * (mSampleBits / 8)) * samples;
        if (mInputStream.eof())
        {
            mInputStream.clear(); // force clear state after eof
        }
        if (mInputStream.seekg(mAudioDataStart + dataOffset, std::ios::end))
            return true;
    }
    return false;
}

bool wave_reader::seek_pcm_cur(int samples)
{
    if (audio_present())
    {
        std::streamoff dataOffset = (mChannelsCount * (mSampleBits / 8)) * samples;
        if (mInputStream.eof())
        {
            mInputStream.clear(); // force clear state after eof
        }
        if (mInputStream.seekg(dataOffset, std::ios::cur))
            return true;
    }
    return false;
}

bool wave_reader::end_of_stream() const
{
    std::streampos currStreampos = mInputStream.tellg();
    return currStreampos >= mAudioDataEnd;
}

bool wave_reader::audio_present() const
{
    return mChannelsCount && mSampleRate && mSampleBits && mSamplesCount;
}

void wave_reader::clear()
{
    mChannelsCount = 0;
    mSampleRate = 0;
    mSampleBits = 0;
    mSamplesCount = 0;
    mAudioDataLength = 0;
    mAudioDataStart = 0;
    mAudioDataEnd = 0;
}

} // namespace cxx