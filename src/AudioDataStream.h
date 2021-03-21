#pragma once

// Interface class of streaming audio data
class AudioDataStream: public cxx::noncopyable
{
public:
    virtual ~AudioDataStream()
    {
    }

    // get media format info
    virtual int GetChannelsCount() const = 0;
    virtual int GetSampleRate() const = 0;
    virtual int GetSampleBits() const = 0;
    virtual int GetSamplesCount() const = 0;

    // reading pcm samples data from stream
    virtual int ReadPCMSamples(int samples, void* buffer) = 0;
    virtual bool SeekPCMFromBeg(int samples) = 0;
    virtual bool SeekPCMFromEnd(int samples) = 0;
    virtual bool SeekPCMFromCur(int samples) = 0;
    virtual bool EndOfStream() const = 0;
};

// open audio stream
extern AudioDataStream* OpenAudioFileStream(const char* fileName);