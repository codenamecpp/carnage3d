#pragma once

#include "AudioArchive.h"

// This class manages in game music and sounds
class AudioManager final: public cxx::noncopyable
{
public:
    AudioArchive mLevelSounds;
    AudioArchive mVoiceSounds;

public:
    bool Initialize();
    void Deinit();

    // Preload sound archives for current level
    bool LoadLevelSounds();
    void FreeLevelSounds();
};

extern AudioManager gAudioManager;