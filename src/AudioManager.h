#pragma once

#include "AudioSampleArchive.h"
#include "SfxDefs.h"
#include "SfxEmitter.h"
#include "AudioDataStream.h"

// This class manages in game music and sounds
class AudioManager final: public cxx::noncopyable
{
    friend class SfxEmitter;

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
    bool StartSound(eSfxSampleType sfxType, SfxSampleIndex sfxIndex, SfxFlags sfxFlags, const glm::vec3& emitterPosition);

    // Get game sound by its identifier, will load audio data if it is not loaded yet
    // @param sfxType: Sound type
    // @param sfxIndex: Sound index
    SfxSample* GetSound(eSfxSampleType sfxType, SfxSampleIndex sfxIndex);

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

    void InitSoundsAndMusicGainValue();
    bool PrepareAudioResources();
    void ShutdownAudioResources();

    void UpdateActiveEmitters();
    void ReleaseActiveEmitters();
    void RegisterActiveEmitter(SfxEmitter* emitter);

    // generate random pitch value
    float NextRandomPitch();

    // music
    bool StartMusic(const char* music);
    void StopMusic();
    void UpdateMusic();
    bool QueueMusicSampleBuffers();

private:
    // constants
    static const int MusicSampleBufferSize = 32768;
    static const int MaxSfxAudioSources = 32;
    static const int MaxMusicSampleBuffers = 4;

    enum eMusicStatus
    {
        eMusicStatus_Stopped,
        eMusicStatus_Playing,
        eMusicStatus_NextTrackRequest,
    };

    // audio resources
    std::vector<AudioSource*> mSfxAudioSources; // available hardware audio sources
    std::vector<SfxSample*> mLevelSfxSamples;
    std::vector<SfxSample*> mVoiceSfxSamples;
    std::vector<SfxEmitter*> mActiveEmitters;
    AudioSource* mMusicAudioSource = nullptr;
    std::deque<AudioSampleBuffer*> mMusicSampleBuffers;

    AudioSampleArchive mLevelSounds;
    AudioSampleArchive mVoiceSounds;

    // music data
    eMusicStatus mMusicStatus = eMusicStatus_NextTrackRequest;
    AudioDataStream* mMusicDataStream = nullptr; // active music stream

    unsigned char mMusicSampleData[MusicSampleBufferSize];

    float mMusicGain = 1.0f;
    float mSoundsGain = 1.0f;

    // object pools
    cxx::object_pool<SfxEmitter> mEmittersPool;
};

extern AudioManager gAudioManager;