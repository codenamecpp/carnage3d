#include "stdafx.h"
#include "SfxEmitter.h"
#include "AudioDevice.h"
#include "AudioManager.h"

SfxSample::SfxSample(eSfxType sfxType, SfxIndex sfxIndex, AudioSampleBuffer* sampleBuffer)
    : mSfxType(sfxType)
    , mSfxIndex(sfxIndex)
    , mSampleBuffer(sampleBuffer)
{
    debug_assert(mSampleBuffer);
}

SfxSample::~SfxSample()
{
    if (mSampleBuffer)
    {
        gAudioDevice.DestroySampleBuffer(mSampleBuffer);
        mSampleBuffer = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////

SfxEmitter::SfxEmitter(GameObject* gameObject, SfxEmitterFlags emitterFlags)
    : mEmitterFlags(emitterFlags)
    , mGameObject(gameObject)
{
}

SfxEmitter::~SfxEmitter()
{
    // stop all active channels
    StopAllSounds();
}

void SfxEmitter::ReleaseEmitter(bool stopSounds)
{
    if (!stopSounds)
    {
        mAudioChannels.clear();
    }
    gAudioManager.DestroyEmitter(this);
}

void SfxEmitter::UpdateEmitterParams(const glm::vec3& emitterPosition)
{
    mEmitterPosition = emitterPosition;

    for (SfxChannel& currChannel: mAudioChannels)
    {
        if (currChannel.mHardwareSource)
        {
            currChannel.mHardwareSource->SetPosition3D(emitterPosition.x, emitterPosition.y, emitterPosition.z);
        }
    }
}

void SfxEmitter::UpdateSounds()
{
    for (SfxChannel& currChannel: mAudioChannels)
    {
        if (currChannel.mHardwareSource == nullptr)
            continue;

        if (currChannel.mHardwareSource->IsStopped())
        {
            currChannel.mHardwareSource = nullptr;
            continue;
        }
    }
}

void SfxEmitter::StopAllSounds()
{
    for (SfxChannel& currChannel: mAudioChannels)
    {
        if (currChannel.mHardwareSource)
        {
            currChannel.mHardwareSource->Stop();
            currChannel.mHardwareSource = nullptr;
        }
    }
}

void SfxEmitter::PauseAllSounds()
{
    for (SfxChannel& currChannel: mAudioChannels)
    {
        if (currChannel.mHardwareSource)
        {
            currChannel.mHardwareSource->Pause();
        }
    }
}

void SfxEmitter::ResumeAllSounds()
{
    for (SfxChannel& currChannel: mAudioChannels)
    {
        if (currChannel.mHardwareSource)
        {
            currChannel.mHardwareSource->Resume();
        }
    }
}

bool SfxEmitter::StartSound(int ichannel, SfxSample* sfxSample, SfxFlags sfxFlags)
{
    if ((ichannel < 0) || (sfxSample == nullptr))
    {
        debug_assert(false);
        return false;
    }

    if (ichannel >= (int) mAudioChannels.size())
    {
        mAudioChannels.resize(ichannel + 1);
    }

    SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        channel.mHardwareSource->Stop();
    }
    if (channel.mHardwareSource == nullptr)
    {
        channel.mHardwareSource = gAudioManager.GetFreeAudioSource();
        if (channel.mHardwareSource == nullptr)
            return false; // out of audio sources
    }

    channel.mSfxFlags = sfxFlags;
    channel.mSfxSample = sfxSample;

    channel.mHardwareSource->Stop();
    if (!channel.mHardwareSource->SetSampleBuffer(sfxSample->mSampleBuffer))
    {
        debug_assert(false);
    }

    float pitchValue = 1.0f;
    if ((sfxFlags & SfxFlags_RandomPitch) > 0)
    {
        pitchValue = gAudioManager.NextRandomPitch();
    }
    else
    {
        pitchValue = channel.mPitchValue;
    }
    if (!channel.mHardwareSource->SetPitch(pitchValue) ||
        !channel.mHardwareSource->SetGain(channel.mGainValue)) 
    {
        debug_assert(false);
    }

    if (!channel.mHardwareSource->SetPosition3D(mEmitterPosition.x, mEmitterPosition.y, mEmitterPosition.z))
    {
        debug_assert(false);
    }

    if (!channel.mHardwareSource->Start((sfxFlags & SfxFlags_Loop) > 0))
    {
        debug_assert(false);
    }
    gAudioManager.RegisterActiveEmitter(this);
    return true;
}

bool SfxEmitter::StopSound(int ichannel)
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        channel.mHardwareSource->Stop();
        channel.mHardwareSource = nullptr;
    }
    return true;
}

bool SfxEmitter::IsPlaying(int ichannel) const
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    const SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        return channel.mHardwareSource->IsPlaying();
    }

    return false;
}

bool SfxEmitter::IsActiveEmitter() const
{
    for (const SfxChannel& currChannel: mAudioChannels)
    {
        if (currChannel.mHardwareSource && currChannel.mHardwareSource)
            return true;
    }
    return false;
}

bool SfxEmitter::IsPaused(int ichannel) const
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    const SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        return channel.mHardwareSource->IsPaused();
    }

    return false;
}

bool SfxEmitter::PauseSound(int ichannel)
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        return channel.mHardwareSource->Pause();
    }

    return false;
}

bool SfxEmitter::ResumeSound(int ichannel)
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        return channel.mHardwareSource->Resume();
    }

    return false;
}

bool SfxEmitter::SetPitch(int ichannel, float pitchValue)
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        if (channel.mPitchValue == pitchValue)
            return true;

        channel.mPitchValue = pitchValue;
        return channel.mHardwareSource->SetPitch(pitchValue);
    }

    return false;
}

bool SfxEmitter::SetGain(int ichannel, float gainValue)
{
    if ((ichannel < 0) || (ichannel >= (int) mAudioChannels.size()))
        return false;

    SfxChannel& channel = mAudioChannels[ichannel];
    if (channel.mHardwareSource)
    {
        if (channel.mGainValue == gainValue)
            return true;

        channel.mGainValue = gainValue;
        return channel.mHardwareSource->SetGain(gainValue);
    }

    return false;
}

bool SfxEmitter::IsAutoreleaseEmitter() const
{
    return (mEmitterFlags & SfxEmitterFlags_Autorelease) > 0;
}
