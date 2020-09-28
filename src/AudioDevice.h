#pragma once

#include "AudioSource.h"
#include "AudioListener.h"
#include "OpenALDefs.h"

// Audio device
class AudioDevice final: public cxx::noncopyable
{
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

    // Destroy virtual audio listener instance
    void DestroyAudioListener(AudioListener* audioListener);

    // Create audio buffer instance
    AudioBuffer* CreateAudioBuffer(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData);
    AudioBuffer* CreateAudioBuffer();

    // Destroy audio buffer instance, it should not be used by anyone at this point
    // @param audioBuffer: Pointer
    void DestroyAudioBuffer(AudioBuffer* audioBuffer);

    // Create audio source instance
    AudioSource* CreateAudioSource();

    // Destroy audio source instance, does not destroys attached audio buffer
    // @param audioInstance: Pointer
    void DestroyAudioSource(AudioSource* audioSource);

private:
    void UpdateSourcesPositions();

private:
    ALCcontext* mContext = nullptr;
    ALCdevice* mDevice = nullptr;
    // allocated objects
    std::vector<AudioListener*> mAllListeners;
    std::vector<AudioSource*> mAllSources;
    std::vector<AudioBuffer*> mAllBuffers;
    // objects pools
    cxx::object_pool<AudioBuffer> mBuffersPool;
    cxx::object_pool<AudioSource> mSourcesPool;
};

extern AudioDevice gAudioDevice;