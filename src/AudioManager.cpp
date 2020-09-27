#include "stdafx.h"
#include "AudioManager.h"
#include "GameMapManager.h"

AudioManager gAudioManager;

bool AudioManager::Initialize()
{
    return true;
}

void AudioManager::Deinit()
{   
    FreeLevelSounds();
}

bool AudioManager::LoadLevelSounds()
{
    FreeLevelSounds();

    gConsole.LogMessage(eLogMessage_Debug, "Loading level sounds...");
    if (!mVoiceSounds.LoadArchive("AUDIO/VOCALCOM"))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load Voice sounds");
    }

    std::string audioBankFileName = cxx::va("AUDIO/LEVEL%03d", gGameMap.mAudioFileNumber); 
    if (!mLevelSounds.LoadArchive(audioBankFileName))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load Level sounds");
    }

    return true;
}

void AudioManager::FreeLevelSounds()
{
    mLevelSounds.FreeArchive();
    mVoiceSounds.FreeArchive();
}
