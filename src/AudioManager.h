#pragma once

#include "SfxArchive.h"
#include "SfxDefs.h"
#include "AudioSource.h"

// This class manages in game music and sounds
class AudioManager final: public cxx::noncopyable
{
public:
    SfxArchive mLevelSounds;
    SfxArchive mVoiceSounds;

public:
    bool Initialize();
    void Deinit();

    void UpdateFrame();

    // Preload sound archives for current level
    bool LoadLevelSounds();
    void FreeLevelSounds();

    // @param sfxIndex: Sound index, one of SfxLevel_*
    AudioSource* PlaySfxLevel(int sfxIndex, const glm::vec3& position, bool enableLoop);

    // @param sfxIndex: Sound index, one of SfxVoice_*
    AudioSource* PlaySfxVoice(int sfxIndex, const glm::vec3& position, bool enableLoop);

    void StopAllSounds();
    void PauseAllSounds();
    void ResumeAllSounds();

private:
    AudioSource* GetFreeSfxAudioSource() const;

    bool AllocateAudioSources();
    void ReleaseAudioSources();

private:
    std::vector<AudioSource*> mAudioSources; // all audio sources
    std::vector<AudioBuffer*> mLevelSoundsBuffers;
    std::vector<AudioBuffer*> mVoiceSoundsBuffers;
};

extern AudioManager gAudioManager;