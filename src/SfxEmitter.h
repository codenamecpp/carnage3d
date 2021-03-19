#pragma once

#include "SfxDefs.h"

//////////////////////////////////////////////////////////////////////////

// Loaded audio effect
class SfxSample
{
    friend class AudioManager;
    friend class SfxChannel;
    friend class SfxEmitter;

public:
    SfxSample(eSfxType sfxType, SfxIndex sfxIndex, AudioSampleBuffer* sampleBuffer);
    ~SfxSample();

private:
    eSfxType mSfxType;
    SfxIndex mSfxIndex;
    AudioSampleBuffer* mSampleBuffer;
};

//////////////////////////////////////////////////////////////////////////

// Audio emitter will emit sounds at specific location
class SfxEmitter final: public cxx::noncopyable
{
    friend class AudioManager;

private:

    //////////////////////////////////////////////////////////////////////////

    // virtual audio channel state
    struct SfxChannel
    {
        AudioSource* mHardwareSource = nullptr; // channel is only active when source set
        SfxSample* mSfxSample = nullptr;
        SfxFlags mSfxFlags = SfxFlags_None;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    SfxEmitter(SfxEmitterFlags emitterFlags);
    ~SfxEmitter();

    // Free emitter
    void ReleaseEmitter(bool stopAudio);

    void UpdateEmitterParams(const glm::vec3& emitterPosition);
    void UpdateSounds(float deltaTime);

    bool StartSound(int ichannel, SfxSample* sfxSample, SfxFlags sfxFlags);
    bool StopSound(int ichannel);
    bool PauseSound(int ichannel);
    bool ResumeSound(int ichannel);
    bool SetPitch(int ichannel, float pitchValue);
    bool SetGain(int ichannel, float gainValue);

    void StopAllSounds();
    void PauseAllSounds();
    void ResumeAllSounds();

    bool IsCurrentlyPlaying(int ichannel) const;
    bool IsCurrentlyPlaying() const;
    bool IsPaused(int ichannel) const;
    
    bool IsAutoreleaseEmitter() const;

private:
    bool CheckForCompletion();

private:
    std::vector<SfxChannel> mAudioChannels;
    glm::vec3 mEmitterPosition;
    SfxEmitterFlags mEmitterFlags = SfxEmitterFlags_None;
};