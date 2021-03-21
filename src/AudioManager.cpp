#include "stdafx.h"
#include "AudioManager.h"
#include "GameMapManager.h"
#include "AudioDevice.h"
#include "CarnageGame.h"
#include "cvars.h"

AudioManager gAudioManager;

//////////////////////////////////////////////////////////////////////////
// cvars

CvarEnum<eGameMusicMode> gCvarGameMusicMode("g_musicMode", eGameMusicMode_Radio, "Game music mode", CvarFlags_Archive);

//////////////////////////////////////////////////////////////////////////

bool AudioManager::Initialize()
{
    if (!PrepareAudioResources())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate audio resources");
        return false;
    }
    
    return true;
}

void AudioManager::Deinit()
{
    StopMusic();
    StopAllSounds();

    ReleaseActiveEmitters();
    ReleaseLevelSounds();

    ShutdownAudioResources();
}

void AudioManager::UpdateFrame()
{
    UpdateActiveEmitters();

    UpdateMusic();
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
    for (AudioSource* source: mSfxAudioSources)
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

void AudioManager::ShutdownAudioResources()
{
    for (AudioSource* currSource: mSfxAudioSources)
    {
        gAudioDevice.DestroyAudioSource(currSource);
    }
    mSfxAudioSources.clear();

    if (mMusicAudioSource)
    {
        gAudioDevice.DestroyAudioSource(mMusicAudioSource);
        mMusicAudioSource = nullptr;
    }

    for (AudioSampleBuffer* currBuffer: mMusicSampleBuffers)
    {
        gAudioDevice.DestroySampleBuffer(currBuffer);
    }
    debug_assert(MaxMusicSampleBuffers == mMusicSampleBuffers.size()); // check for leaks
    mMusicSampleBuffers.clear();
}

AudioSource* AudioManager::GetFreeAudioSource() const
{
    for (AudioSource* currSource: mSfxAudioSources)
    {
        if (currSource && !currSource->IsPlaying() && !currSource->IsPaused())
            return currSource;
    }

    return nullptr;
}

bool AudioManager::PrepareAudioResources()
{
    for (int icurr = 0; icurr < MaxSfxAudioSources; ++icurr)
    {
        AudioSource* audioSource = gAudioDevice.CreateAudioSource();
        if (audioSource == nullptr)
            break;

        mSfxAudioSources.push_back(audioSource);
    }

    // allocate additional music source
    mMusicAudioSource = gAudioDevice.CreateAudioSource();
    debug_assert(mMusicAudioSource);

    if (mMusicAudioSource)
    {
        // todo: move to settings
        mMusicAudioSource->SetGain(0.3f);
    }

    // allocate music sample buffers
    for (int icurr = 0; icurr < MaxMusicSampleBuffers; ++icurr)
    {
        AudioSampleBuffer* sampleBuffer = gAudioDevice.CreateSampleBuffer();
        if (sampleBuffer == nullptr)
            break;

        mMusicSampleBuffers.push_back(sampleBuffer);
    }

    return true;
}

void AudioManager::StopAllSounds()
{
    for (AudioSource* currSource: mSfxAudioSources)
    {
        if (currSource->IsPlaying() || currSource->IsPaused())
        {
            currSource->Stop();
        }
    }
}

void AudioManager::PauseAllSounds()
{
    for (AudioSource* currSource: mSfxAudioSources)
    {
        if (currSource->IsPlaying())
        {
            currSource->Pause();
        }
    }
}

void AudioManager::ResumeAllSounds()
{
    for (AudioSource* currSource: mSfxAudioSources)
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

SfxEmitter* AudioManager::CreateEmitter(GameObject* gameObject, const glm::vec3& emitterPosition, SfxEmitterFlags emitterFlags)
{
    SfxEmitter* emitter = mEmittersPool.create(gameObject, emitterFlags);
    debug_assert(emitter);
    emitter->UpdateEmitterParams(emitterPosition);
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
    cxx::erase_elements(mActiveEmitters, sfxEmitter);
}

void AudioManager::ReleaseActiveEmitters()
{
    if (mActiveEmitters.empty())
        return;

    std::vector<SfxEmitter*> activeEmitters;
    std::vector<SfxEmitter*> deleteEmitters;
    activeEmitters.swap(mActiveEmitters);

    for (SfxEmitter* currEmitter: activeEmitters)
    {
        currEmitter->StopAllSounds();
        if (currEmitter->IsAutoreleaseEmitter())
        {
            deleteEmitters.push_back(currEmitter);
        }
    }
    // destroy autorelease emitters
    for (SfxEmitter* currEmitter: deleteEmitters)
    {
        mEmittersPool.destroy(currEmitter);
    }
}

void AudioManager::UpdateActiveEmitters()
{
    if (mActiveEmitters.empty())
        return;

    std::vector<SfxEmitter*> inactiveEmitters;
    for (SfxEmitter* currEmitter: mActiveEmitters)
    {
        if (currEmitter->mGameObject) // sync audio params
        {
            glm::vec3 gameObjectPosition = currEmitter->mGameObject->GetPosition();
            currEmitter->UpdateEmitterParams(gameObjectPosition);
        }

        currEmitter->UpdateSounds();
        if (!currEmitter->IsActiveEmitter())
        {
            inactiveEmitters.push_back(currEmitter);
        }
    }

    for (SfxEmitter* currEmitter: inactiveEmitters)
    {
        cxx::erase_elements(mActiveEmitters, currEmitter);
        if (currEmitter->IsAutoreleaseEmitter())
        {
            mEmittersPool.destroy(currEmitter);
        }
    }
}

bool AudioManager::StartSound(eSfxType sfxType, SfxIndex sfxIndex, SfxFlags sfxFlags, const glm::vec3& emitterPosition)
{
    SfxSample* audioSample = GetSound(sfxType, sfxIndex);
    if (audioSample == nullptr)
        return false;

    SfxEmitter* autoreleaseEmitter = CreateEmitter(nullptr, emitterPosition, SfxEmitterFlags_Autorelease);
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

void AudioManager::RegisterActiveEmitter(SfxEmitter* emitter)
{
    if (emitter == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (!cxx::contains(mActiveEmitters, emitter))
    {
        mActiveEmitters.push_back(emitter);
    }
}

void AudioManager::UpdateMusic()
{
    if ((gCvarGameMusicMode.mValue == eGameMusicMode_Disabled) || (mMusicStatus == eMusicStatus_Stopped))
        return;

    if (mMusicStatus == eMusicStatus_NextTrackRequest)
    {
        StartMusic("MUSIC/Track11"); // ambient 
        return;
    }

    if (mMusicStatus == eMusicStatus_Playing)
    {
        // process buffers
        std::vector<AudioSampleBuffer*> sampleBuffers;
        if (!mMusicAudioSource->ProcessBuffersQueue(sampleBuffers))
        {
            StopMusic();
            return;
        }
        if (!sampleBuffers.empty())
        {
            mMusicSampleBuffers.insert(mMusicSampleBuffers.end(), sampleBuffers.begin(), sampleBuffers.end());
        }

        if (!QueueMusicSampleBuffers())
        {
            if (mMusicDataStream->EndOfStream())
            {
                StopMusic();
                mMusicStatus = eMusicStatus_NextTrackRequest;
                return;
            }
        }

        if (!mMusicAudioSource->IsPlaying() && !mMusicAudioSource->Start())
        {
            StopMusic();
            return;
        }

        return;
    }
}

bool AudioManager::StartMusic(const char* music)
{
    StopMusic();

    if (mMusicAudioSource == nullptr)
        return false;

    mMusicDataStream = OpenAudioFileStream(music);
    if (mMusicDataStream == nullptr)
        return false;

    if (!QueueMusicSampleBuffers())
    {
        SafeDelete(mMusicDataStream);
        return false;
    }

    if (!mMusicAudioSource->Start())
    {
        StopMusic();
        return false;
    }

    mMusicStatus = eMusicStatus_Playing;
    return true;
}

void AudioManager::StopMusic()
{
    mMusicStatus = eMusicStatus_Stopped;

    if (mMusicAudioSource)
    {
        std::vector<AudioSampleBuffer*> sampleBuffers;
        mMusicAudioSource->Stop();
        mMusicAudioSource->ProcessBuffersQueue(sampleBuffers);
        if (!sampleBuffers.empty())
        {
            mMusicSampleBuffers.insert(mMusicSampleBuffers.end(), sampleBuffers.begin(), sampleBuffers.end());
        }
    }

    SafeDelete(mMusicDataStream);
}

bool AudioManager::QueueMusicSampleBuffers()
{
    debug_assert(mMusicDataStream);
    debug_assert(mMusicAudioSource);

    int bytesPerSample = mMusicDataStream->GetChannelsCount() * (mMusicDataStream->GetSampleBits() / 8);
    int samplesPerBuffer = MusicSampleBufferSize / bytesPerSample;
    // fill sample buffers
    int totalSamplesProcessed = 0;
    for (;;)
    {
        if (mMusicSampleBuffers.empty())
            break;

        int numSamplesRead = mMusicDataStream->ReadPCMSamples(samplesPerBuffer, mMusicSampleData);
        if (numSamplesRead == 0)
            break;

        int dataLength = numSamplesRead * bytesPerSample;
        totalSamplesProcessed += numSamplesRead;

        AudioSampleBuffer* sampleBuffer = mMusicSampleBuffers.front();
        if (!sampleBuffer->SetupBufferData(mMusicDataStream->GetSampleRate(), 
            mMusicDataStream->GetSampleBits(), 
            mMusicDataStream->GetChannelsCount(), dataLength, mMusicSampleData))
        {
            debug_assert(false);
            break;
        }
        if (!mMusicAudioSource->QueueSampleBuffer(sampleBuffer))
        {
            debug_assert(false);
            break;
        }

        mMusicSampleBuffers.pop_front();
    }

    return totalSamplesProcessed > 0;
}
