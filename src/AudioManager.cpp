#include "stdafx.h"
#include "AudioManager.h"
#include "GameMapManager.h"
#include "AudioDevice.h"
#include "CarnageGame.h"

AudioManager gAudioManager;

bool AudioManager::Initialize()
{
    if (!AllocateAudioSources())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate audio sources");
        return false;
    }

    return true;
}

void AudioManager::Deinit()
{
    StopAllSounds();

    ReleaseSoundEmitters();
    ReleaseAudioSources();
    ReleaseLevelSounds();
}

void AudioManager::UpdateFrame()
{
    UpdateSoundEmitters();
}

bool AudioManager::PreloadLevelSounds()
{
    ReleaseLevelSounds();

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

    mLevelSfxSamples.resize(mLevelSounds.GetEntriesCount());
    mVoiceSfxSamples.resize(mVoiceSounds.GetEntriesCount());

    return true;
}

void AudioManager::ReleaseLevelSounds()
{
    // stop all sources and detach buffers
    for (AudioSource* source: mAudioSources)
    {
        source->Stop();
        source->SetSampleBuffer(nullptr);
    }

    mLevelSounds.FreeArchive();
    mVoiceSounds.FreeArchive();

    for (SfxSample* currSample: mLevelSfxSamples)
    {
        SafeDelete(currSample);
    }

    for (SfxSample* currSample: mVoiceSfxSamples)
    {
        SafeDelete(currSample);
    }

    mLevelSfxSamples.clear();
    mVoiceSfxSamples.clear();
}

void AudioManager::ReleaseAudioSources()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource)
        {
            gAudioDevice.DestroyAudioSource(currSource);
        }
    }
    mAudioSources.clear();
}

AudioSource* AudioManager::GetFreeAudioSource() const
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource && !currSource->IsPlaying() && !currSource->IsPaused())
            return currSource;
    }

    return nullptr;
}

bool AudioManager::AllocateAudioSources()
{
    const int MaxAudioSources = 32;
    for (int icurr = 0; icurr < MaxAudioSources; ++icurr)
    {
        AudioSource* audioSource = gAudioDevice.CreateAudioSource();
        if (audioSource == nullptr)
            break;

        mAudioSources.push_back(audioSource);
    }
    return true;
}

void AudioManager::StopAllSounds()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource->IsPlaying() || currSource->IsPaused())
        {
            currSource->Stop();
        }
    }
}

void AudioManager::PauseAllSounds()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource->IsPlaying())
        {
            currSource->Pause();
        }
    }
}

void AudioManager::ResumeAllSounds()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource->IsPaused())
        {
            currSource->Resume();
        }
    }
}

SfxSample* AudioManager::GetSound(eSfxType sfxType, SfxIndex sfxIndex)
{
    AudioSampleArchive& sampleArchive = (sfxType == eSfxType_Level) ? mLevelSounds : mVoiceSounds;
    if ((int) sfxIndex >= sampleArchive.GetEntriesCount())
    {
        debug_assert(false);
        return nullptr;
    }

    std::vector<SfxSample*>& samples = (sfxType == eSfxType_Level) ? 
        mLevelSfxSamples : 
        mVoiceSfxSamples;

    if (samples[sfxIndex] == nullptr)
    {
        AudioSampleArchive::SampleEntry archiveEntry;
        if (!sampleArchive.GetEntryData(sfxIndex, archiveEntry))
        {
            debug_assert(false);
            return nullptr;
        }
        // upload audio data
        AudioSampleBuffer* audioBuffer = gAudioDevice.CreateSampleBuffer(
            archiveEntry.mSampleRate,
            archiveEntry.mBitsPerSample,
            archiveEntry.mChannelsCount,
            archiveEntry.mDataLength,
            archiveEntry.mData);
        debug_assert(audioBuffer && !audioBuffer->IsBufferError());

        // free source data
        sampleArchive.FreeEntryData(sfxIndex);

        if (audioBuffer)
        {
            samples[sfxIndex] = new SfxSample(sfxType, sfxIndex, audioBuffer);
        }
    }
    return samples[sfxIndex];
}

SfxEmitter* AudioManager::CreateEmitter(const glm::vec3& emitterPosition, SfxEmitterFlags emitterFlags)
{
    SfxEmitter* emitter = mEmittersPool.create(emitterFlags);
    debug_assert(emitter);
    emitter->UpdateEmitterParams(emitterPosition);
    mAllEmitters.push_back(emitter);
    return emitter;
}

void AudioManager::DestroyEmitter(SfxEmitter* sfxEmitter)
{
    if (sfxEmitter == nullptr)
    {
        debug_assert(false);
        return;
    }

    mEmittersPool.destroy(sfxEmitter);
    cxx::erase_elements(mAllEmitters, sfxEmitter);
}

void AudioManager::ReleaseSoundEmitters()
{
    for (SfxEmitter* currEmitter: mAllEmitters)
    {
        mEmittersPool.destroy(currEmitter);
    }
    mAllEmitters.clear();
}

void AudioManager::UpdateSoundEmitters()
{
    std::vector<SfxEmitter*> deleteEmitters;
    for (SfxEmitter* currEmitter: mAllEmitters)
    {
        if (currEmitter->CheckForCompletion())
        {
            deleteEmitters.push_back(currEmitter);
        }
    }
    // destroy dead emitters
    for (SfxEmitter* currEmitter: deleteEmitters)
    {
        DestroyEmitter(currEmitter);
    }
}

bool AudioManager::StartSound(eSfxType sfxType, SfxIndex sfxIndex, SfxFlags sfxFlags, const glm::vec3& emitterPosition)
{
    SfxSample* audioSample = GetSound(sfxType, sfxIndex);
    if (audioSample == nullptr)
        return false;

    SfxEmitter* autoreleaseEmitter = CreateEmitter(emitterPosition, SfxEmitterFlags_Autorelease);
    debug_assert(autoreleaseEmitter);
    if (autoreleaseEmitter)
    {
        if (autoreleaseEmitter->StartSound(0, audioSample, sfxFlags))
            return true;

        DestroyEmitter(autoreleaseEmitter);
    }
    return false;
}

float AudioManager::NextRandomPitch()
{
    static const float _pitchValues[] = {0.95f, 1.0f, 1.1f};

    int randomIndex = gCarnageGame.mGameRand.generate_int() % CountOf(_pitchValues);
    return _pitchValues[randomIndex];
}
