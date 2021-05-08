#pragma once

#include "SfxDefs.h"

// forwards
class GameObject;

//////////////////////////////////////////////////////////////////////////

// Loaded audio effect
class SfxSample
{
    friend class AudioManager;
    friend class SfxChannel;
    friend class SfxEmitter;

public:
    SfxSample(eSfxSampleType sfxType, SfxSampleIndex sfxIndex, AudioSampleBuffer* sampleBuffer);
    ~SfxSample();

private:
    eSfxSampleType mSfxType;
    SfxSampleIndex mSfxIndex;
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
        // audio params
        float mPitchValue = 1.0f;
        float mGainValue = 1.0f;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    // readonly
    GameObject* mGameObject = nullptr; // optional

public:
    SfxEmitter(GameObject* gameObject, SfxEmitterFlags emitterFlags);
    ~SfxEmitter();

    // Free emitter
    void ReleaseEmitter(bool stopSounds);

    void UpdateEmitterParams(const glm::vec3& emitterPosition);
    void UpdateSounds();

    void StopAllSounds();
    void PauseAllSounds();
    void ResumeAllSounds();

    bool StartSound(int ichannel, SfxSample* sfxSample, SfxFlags sfxFlags);
    bool StopSound(int ichannel);
    bool PauseSound(int ichannel);
    bool ResumeSound(int ichannel);
    bool SetPitch(int ichannel, float pitchValue);
    bool SetGain(int ichannel, float gainValue);

    bool IsPlaying(int ichannel) const;
    bool IsPaused(int ichannel) const;
    
    bool IsAutoreleaseEmitter() const;
    bool IsActiveEmitter() const;

private:
    std::vector<SfxChannel> mAudioChannels;
    glm::vec3 mEmitterPosition;
    SfxEmitterFlags mEmitterFlags = SfxEmitterFlags_None;
};