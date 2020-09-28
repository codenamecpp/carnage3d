#pragma once

// Contains sample data buffer and media properties
class AudioBuffer final: public cxx::noncopyable
{
    friend class AudioSource;

public:
    AudioBuffer();
    ~AudioBuffer();

    // Setup audio format and sample data
    bool SetupBufferData(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData);
    bool IsBufferError() const;

private:
    unsigned int mBufferID = 0; // openal buffer handle
};

// Represents audio channel instance which can play sounds
class AudioSource final: public cxx::noncopyable
{
    friend class AudioDevice;

public:
    // readonly
    AudioBuffer* mAudioBuffer = nullptr;

public:
    AudioSource();
    ~AudioSource();

    // Replace current audio buffer, will stop playing sound
    // @param audioBuffer: New audio buffer or nullptr 
    bool SetupSourceBuffer(AudioBuffer* audioBuffer);

    // Control audio state
    bool Start(bool enableLoop = false);
    bool Stop();
    bool Pause();
    bool Resume();
    bool SetLoop(bool enableLoop);
    // Set audio params
    bool SetVolume(float value);
    bool SetPitch(float value);
    bool SetPosition3D(float positionx, float positiony, float positionz);
    // Get current audio state
    bool IsPlaying() const;
    bool IsStopped() const;
    bool IsPaused() const;
    bool IsSourceError() const;

private:
    unsigned int mSourceID = 0; // openal source handle

    glm::vec3 mSourceLocation;
};