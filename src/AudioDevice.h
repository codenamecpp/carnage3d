#pragma once

#include "AudioSource.h"
#include "AudioListener.h"
#include "OpenALDefs.h"

// Hardware audio device
class AudioDevice final: public cxx::noncopyable
{
    friend class AudioSource;

public:
    // readonly
    AudioDeviceCaps mDeviceCaps;

public:
    ~AudioDevice();

    // Setup audio device internal resources
    bool Initialize();
    void Deinit();
    bool IsInitialized() const;

    // Update active audio sources
    void UpdateFrame();

    // Setup device params
    bool SetMasterVolume(float gainValue);

    // Create virtual audio listener instance
    AudioListener* CreateAudioListener();

    // Create sample buffer instance
    AudioSampleBuffer* CreateSampleBuffer(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData);
    AudioSampleBuffer* CreateSampleBuffer();

    // Create audio source instance
    AudioSource* CreateAudioSource();

    // Free virtual audio listener instance
    void DestroyAudioListener(AudioListener* audioListener);

    // Free sample buffer instance, it should not be used by anyone at this point
    // @param audioBuffer: Pointer
    void DestroySampleBuffer(AudioSampleBuffer* audioBuffer);

    // Free audio source instance, does not destroys attached audio buffer
    // @param audioInstance: Pointer
    void DestroyAudioSource(AudioSource* audioSource);

private:
    void QueryAudioDeviceCaps();
    void UpdateSourcesPositions();

    // Find sample buffer by internal identifier
    AudioSampleBuffer* GetSampleBufferWithID(unsigned int bufferID) const;

    // Find source by internal identifier
    AudioSource* GetAudioSourceWithID(unsigned int sourceID) const;

private:
    ALCcontext* mContext = nullptr;
    ALCdevice* mDevice = nullptr;
    // allocated objects
    std::vector<AudioListener*> mAllListeners;
    std::vector<AudioSource*> mAllSources;
    std::vector<AudioSampleBuffer*> mAllBuffers;
    // objects pools
    cxx::object_pool<AudioSampleBuffer> mBuffersPool;
    cxx::object_pool<AudioSource> mSourcesPool;
};

extern AudioDevice gAudioDevice;