#pragma once

// forwards
class AudioListener;

// Audio source status
enum eAudioSourceStatus
{
    eAudioSourceStatus_Stopped,
    eAudioSourceStatus_Playing,
    eAudioSourceStatus_Paused,
};

// Audio source type
enum eAudioSourceType
{
    eAudioSourceType_Static,
    eAudioSourceType_Streaming,
};

// Contains sample data buffer and media properties
class AudioSampleBuffer final: public cxx::noncopyable
{
    friend class AudioSource;
    friend class AudioDevice;

public:
    AudioSampleBuffer();
    ~AudioSampleBuffer();

    // Setup audio format and sample data
    bool SetupBufferData(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData);
    bool IsBufferError() const;

    // Sample data desc
    bool IsStereo() const;
    bool IsMono() const;

private:
    unsigned int mBufferID = 0; // openal buffer handle
    int mSampleRate = 0;
    int mBitsPerSample = 0;
    int mChannelsCount = 0;
};

// Represents audio channel instance which can play sounds
class AudioSource final: public cxx::noncopyable
{
    friend class AudioDevice;

public:
    AudioSource();
    ~AudioSource();

    // Attach sample buffer, will stop playing current sound, set source to static type
    // @param audioBuffer: New audio buffer or nullptr 
    bool SetSampleBuffer(AudioSampleBuffer* audioBuffer);

    // Queue sample buffer, set source to streaming type
    bool QueueSampleBuffer(AudioSampleBuffer* audioBuffer);
    // Unqueue sample buffers which was already processed, works for streaming type only
    // @param audioBuffers: Output processed sample buffers
    bool ProcessBuffersQueue(std::vector<AudioSampleBuffer*>& audioBuffers);

    // Control audio state
    bool Start(bool enableLoop = false);
    bool Stop();
    bool Pause();
    bool Resume();
    bool SetLoop(bool enableLoop);
    // Set audio params
    bool SetGain(float value);
    bool SetPitch(float value);
    bool SetPosition3D(float positionx, float positiony, float positionz);
    bool SetVelocity3D(float velocityx, float velocityy, float velocityz);
    // Get source current status
    eAudioSourceStatus GetSourceStatus() const;
    // Status shortcuts
    bool IsPlaying() const
    {
        return GetSourceStatus() == eAudioSourceStatus_Playing;
    }
    bool IsStopped() const
    {
        return GetSourceStatus() == eAudioSourceStatus_Stopped;
    }
    bool IsPaused() const
    {
        return GetSourceStatus() == eAudioSourceStatus_Paused;
    }

    // Get source current type
    eAudioSourceType GetSourceType() const;
    // Source type shortcuts
    bool IsSourceStatic() const
    {
        return GetSourceType() == eAudioSourceType_Static;
    }
    bool IsSourceStreaming() const
    {
        return GetSourceType() == eAudioSourceType_Streaming;
    }

    bool IsSourceError() const;

private:
    unsigned int mSourceID = 0; // openal source handle

    glm::vec3 mSourceLocation;
};