#pragma once

#include "AudioSampleArchive.h"
#include "SfxDefs.h"
#include "SfxEmitter.h"

// This class manages in game music and sounds
class AudioManager final: public cxx::noncopyable
{
    friend class SfxEmitter;

public:
    AudioSampleArchive mLevelSounds;
    AudioSampleArchive mVoiceSounds;

public:
    bool Initialize();
    void Deinit();

    void UpdateFrame();

    // Preload sound archives for current level
    bool PreloadLevelSounds();
    void ReleaseLevelSounds();

    // Simple play one shot sound within world
    // @param sfxType, sfxIndex: Sound identifier
    // @param emitterPosition: Sound position
    bool StartSound(eSfxType sfxType, SfxIndex sfxIndex, SfxFlags sfxFlags, const glm::vec3& emitterPosition);

    // Get game sound by its identifier, will load audio data if it is not loaded yet
    // @param sfxType: Sound type
    // @param sfxIndex: Sound index
    SfxSample* GetSound(eSfxType sfxType, SfxIndex sfxIndex);

    // Allocate new sound emitter instance
    // @param gameObject: Game object which emitting sounds, optional
    SfxEmitter* CreateEmitter(GameObject* gameObject, const glm::vec3& emitterPosition, SfxEmitterFlags emitterFlags = SfxEmitterFlags_None);

    // Free emitter instance and stop all its active sounds
    void DestroyEmitter(SfxEmitter* sfxEmitter);

    void StopAllSounds();
    void PauseAllSounds();
    void ResumeAllSounds();

private:
    AudioSource* GetFreeAudioSource() const;

    bool AllocateAudioSources();
    void ReleaseAudioSources();

    void UpdateActiveEmitters();
    void ReleaseActiveEmitters();
    void RegisterActiveEmitter(SfxEmitter* emitter);

    // generate random pitch value
    float NextRandomPitch();

private:
    std::vector<AudioSource*> mAudioSources; // all available hardware audio sources
    std::vector<SfxSample*> mLevelSfxSamples;
    std::vector<SfxSample*> mVoiceSfxSamples;
    std::vector<SfxEmitter*> mActiveEmitters;

    cxx::object_pool<SfxEmitter> mEmittersPool;
};

extern AudioManager gAudioManager;