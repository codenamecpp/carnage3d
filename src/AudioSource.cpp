#include "stdafx.h"
#include "AudioSource.h"
#include "OpenALDefs.h"

AudioBuffer::AudioBuffer()
{
    ::alGenBuffers(1, &mBufferID);
    alCheckError();
}

AudioBuffer::~AudioBuffer()
{
    if (::alIsBuffer(mBufferID))
    {
        ::alDeleteBuffers(1, &mBufferID);
        alCheckError();
    }
}

bool AudioBuffer::SetupBufferData(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData)
{
    if (::alIsBuffer(mBufferID))
    {
        if ((bitsPerSample != 8) && (bitsPerSample != 16))
        {
            debug_assert(false);
            return false;
        }
        if ((channelsCount != 1) && (channelsCount != 2))
        {
            debug_assert(false);
            return false;
        }

        ALenum alFormat = (channelsCount == 1) ? 
            (bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) : 
            (bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);

        ::alBufferData(mBufferID, alFormat, bufferData, dataLength, sampleRate);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioBuffer::IsBufferError() const
{
    return ::alIsBuffer(mBufferID) == AL_FALSE;
}

//////////////////////////////////////////////////////////////////////////

AudioSource::AudioSource()
{
    ::alGenSources(1, &mSourceID);
    alCheckError();

    ::alSourcef(mSourceID, AL_PITCH, 1.0f);
    alCheckError();

    ::alSourcef(mSourceID, AL_GAIN, 1.0f);
    alCheckError();

    ::alSource3f(mSourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alCheckError();

    ::alSource3f(mSourceID, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alCheckError();

    ::alSourcei(mSourceID, AL_LOOPING, AL_FALSE);
    alCheckError();
}

AudioSource::~AudioSource()
{
    Stop();

    if (::alIsSource(mSourceID))
    {
        ::alDeleteSources(1, &mSourceID);
        alCheckError();
    }
}

bool AudioSource::SetupSourceBuffer(AudioBuffer* audioBuffer)
{
    if (mAudioBuffer == audioBuffer)
        return true;

    if (::alIsSource(mSourceID))
    {
        ::alSourceStop(mSourceID);
        alCheckError();

        ::alSourcei(mSourceID, AL_BUFFER, 0);
        alCheckError();
    }
    mAudioBuffer = audioBuffer;
    if (mAudioBuffer)
    {
        ::alSourcei(mSourceID, AL_BUFFER, mAudioBuffer->mBufferID);
        alCheckError();
    }

    return true;
}

bool AudioSource::Start(bool enableLoop)
{
    debug_assert(mAudioBuffer);

    if (::alIsSource(mSourceID))
    {
        ::alSourcePlay(mSourceID);
        alCheckError();

        ::alSourcei(mSourceID, AL_LOOPING, enableLoop ? AL_TRUE : AL_FALSE);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::Stop()
{
    if (::alIsSource(mSourceID))
    {
        ::alSourceStop(mSourceID);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::Pause()
{
    if (::alIsSource(mSourceID))
    {
        ::alSourcePause(mSourceID);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::Resume()
{
    if (::alIsSource(mSourceID))
    {
        ::alSourcePlay(mSourceID);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::SetLoop(bool enableLoop)
{
    if (::alIsSource(mSourceID))
    {
        ::alSourcei(mSourceID, AL_LOOPING, enableLoop ? AL_TRUE : AL_FALSE);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::SetVolume(float value)
{
    if (::alIsSource(mSourceID))
    {
        ::alSourcef(mSourceID, AL_GAIN, value);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::SetPitch(float value)
{
    if (::alIsSource(mSourceID))
    {
        ::alSourcef(mSourceID, AL_PITCH, value);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::SetPosition3D(float positionx, float positiony, float positionz)
{
    if (::alIsSource(mSourceID))
    {
        mSourceLocation.x = positionx;
        mSourceLocation.y = positiony;
        mSourceLocation.z = positionz;

        // position will be updated on next audio device update frame
        return true;
    }
    return false;
}

bool AudioSource::IsPlaying() const
{
    if (::alIsSource(mSourceID))
    {
        ALint currentState = AL_STOPPED;
        ::alGetSourcei(mSourceID, AL_SOURCE_STATE, &currentState);
        alCheckError();

        return currentState == AL_PLAYING;
    }
    return false;
}

bool AudioSource::IsStopped() const
{
    if (::alIsSource(mSourceID))
    {
        ALint currentState = AL_STOPPED;
        ::alGetSourcei(mSourceID, AL_SOURCE_STATE, &currentState);
        alCheckError();

        return currentState == AL_STOPPED;
    }
    return false;
}

bool AudioSource::IsPaused() const
{
    if (::alIsSource(mSourceID))
    {
        ALint currentState = AL_STOPPED;
        ::alGetSourcei(mSourceID, AL_SOURCE_STATE, &currentState);
        alCheckError();

        return currentState == AL_PAUSED;
    }
    return false;
}

bool AudioSource::IsSourceError() const
{
    return ::alIsSource(mSourceID) == AL_FALSE;
}