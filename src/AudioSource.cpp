#include "stdafx.h"
#include "AudioSource.h"
#include "OpenALDefs.h"
#include "AudioDevice.h"

AudioSampleBuffer::AudioSampleBuffer()
{
    ::alGenBuffers(1, &mBufferID);
    alCheckError();
}

AudioSampleBuffer::~AudioSampleBuffer()
{
    if (::alIsBuffer(mBufferID))
    {
        ::alDeleteBuffers(1, &mBufferID);
        alCheckError();
    }
}

bool AudioSampleBuffer::SetupBufferData(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData)
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

        mDataLength = dataLength;
        mSampleRate = sampleRate;
        mBitsPerSample = bitsPerSample;
        mChannelsCount = channelsCount;

        ALenum alFormat = (channelsCount == 1) ? 
            (bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) : 
            (bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);

        ::alBufferData(mBufferID, alFormat, bufferData, dataLength, sampleRate);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSampleBuffer::IsBufferError() const
{
    return ::alIsBuffer(mBufferID) == AL_FALSE;
}

bool AudioSampleBuffer::IsMono() const
{
    return mChannelsCount == 1;
}

bool AudioSampleBuffer::IsStereo() const
{
    return mChannelsCount == 2;
}

float AudioSampleBuffer::GetBufferDurationSeconds() const
{
    float durationSeconds = 0.0f;
    if (ContainsData())
    {
        int samplesCount = mDataLength / (mChannelsCount * (mBitsPerSample / 8));
        durationSeconds = (1.0f * samplesCount) / mSampleRate;
    }
    return durationSeconds;
}

bool AudioSampleBuffer::ContainsData() const
{
    return (mDataLength > 0) && (mChannelsCount > 0) && (mBitsPerSample > 0) && (mSampleRate > 0);
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

bool AudioSource::SetSampleBuffer(AudioSampleBuffer* audioBuffer)
{
    unsigned int bufferID = audioBuffer ? audioBuffer->mBufferID : 0;

    if (::alIsSource(mSourceID))
    {
        ::alSourceStop(mSourceID);
        alCheckError();

        ::alSourcei(mSourceID, AL_BUFFER, 0);
        alCheckError();

        ::alSourcei(mSourceID, AL_BUFFER, bufferID);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::QueueSampleBuffer(AudioSampleBuffer* audioBuffer)
{
    if (audioBuffer == nullptr)
    {
        debug_assert(false);
        return false;
    }

    if (::alIsSource(mSourceID))
    {
        ALint currentSourceType = AL_UNDETERMINED;
        ::alGetSourcei(mSourceID, AL_SOURCE_TYPE, &currentSourceType);
        alCheckError();

        if (currentSourceType != AL_STREAMING)
        {
            ::alSourceStop(mSourceID);
            alCheckError();

            ::alSourcei(mSourceID, AL_BUFFER, 0);
            alCheckError();
        }

        ::alSourceQueueBuffers(mSourceID, 1, &audioBuffer->mBufferID);
        alCheckError();

        return true;
    }
    return false;
}

bool AudioSource::ProcessBuffersQueue(std::vector<AudioSampleBuffer*>& audioBuffers)
{
    if (::alIsSource(mSourceID))
    {
        ALint currentSourceType = AL_UNDETERMINED;
        ::alGetSourcei(mSourceID, AL_SOURCE_TYPE, &currentSourceType);
        alCheckError();

        if (currentSourceType != AL_STREAMING)
        {
            debug_assert(currentSourceType == AL_UNDETERMINED);
            return false;
        }

        int numBuffersProcessed = 0;
        ::alGetSourcei(mSourceID, AL_BUFFERS_PROCESSED, &numBuffersProcessed);
        alCheckError();

        for (int icurr = 0; icurr < numBuffersProcessed; ++icurr)
        {
            ALuint bufferID = 0;
            ::alSourceUnqueueBuffers(mSourceID, 1, &bufferID);
            alCheckError();

            AudioSampleBuffer* sampleBuffer = gAudioDevice.GetSampleBufferWithID(bufferID);
            debug_assert(sampleBuffer);
            if (sampleBuffer)
            {
                audioBuffers.push_back(sampleBuffer);
            }
        }

        return true;
    }
    return false;
}

bool AudioSource::Start(bool enableLoop)
{
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

bool AudioSource::SetGain(float value)
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

bool AudioSource::IsSourceError() const
{
    return ::alIsSource(mSourceID) == AL_FALSE;
}

bool AudioSource::SetVelocity3D(float velocityx, float velocityy, float velocityz)
{
    if (::alIsSource(mSourceID))
    {
        ::alSource3f(mSourceID, AL_VELOCITY, velocityx, velocityy, velocityz);
        alCheckError();

        return true;
    }
    return false;
}

eAudioSourceStatus AudioSource::GetSourceStatus() const
{
    if (::alIsSource(mSourceID))
    {
        ALint currentState = AL_STOPPED;
        ::alGetSourcei(mSourceID, AL_SOURCE_STATE, &currentState);
        alCheckError();

        if (currentState == AL_PLAYING)
            return eAudioSourceStatus_Playing;

        if (currentState == AL_PAUSED)
            return eAudioSourceStatus_Paused;
    }

    return eAudioSourceStatus_Stopped;
}

eAudioSourceType AudioSource::GetSourceType() const
{
    if (::alIsSource(mSourceID))
    {
        ALint currentSourceType = AL_UNDETERMINED;
        ::alGetSourcei(mSourceID, AL_SOURCE_TYPE, &currentSourceType);
        alCheckError();

        if (currentSourceType == AL_STREAMING)
            return eAudioSourceType_Streaming;

        return eAudioSourceType_Static;
    }

    return eAudioSourceType_Static;
}
