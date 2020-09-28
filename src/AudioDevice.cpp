#include "stdafx.h"
#include "AudioDevice.h"
#include "OpenALDefs.h"

AudioDevice gAudioDevice;

AudioDevice::~AudioDevice()
{
    mBuffersPool.cleanup();
    mSourcesPool.cleanup();
}

bool AudioDevice::Initialize()
{
    gConsole.LogMessage(eLogMessage_Debug, "Audio device initialization...");

    mDevice = ::alcOpenDevice(nullptr);
    if (mDevice == nullptr)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open audio device");
        return false;
    }

    gConsole.LogMessage(eLogMessage_Info, "Audio device description: %s", alcGetString(mDevice, ALC_DEVICE_SPECIFIER));

    mContext = ::alcCreateContext(mDevice, nullptr);
    if (mContext == nullptr)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create audio device context");

        Deinit();
        return false;
    }
    //alClearError();
    if (::alcMakeContextCurrent(mContext))
    {
        // setup default listener params
        ::alListener3f(AL_POSITION, 0.0f, 0.0f, 1.0f);
        alCheckError();

        ::alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alCheckError();

        ::alListenerf(AL_GAIN, 1.0f);
        alCheckError();


        float orientation_at_up[] = {0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f};
        ::alListenerfv(AL_ORIENTATION, orientation_at_up);
        alCheckError();

        return true;
    }

    gConsole.LogMessage(eLogMessage_Warning, "Audio device context error");
    return false;
}

void AudioDevice::Deinit()
{
    // destroy sources
    for (AudioSource* currSource: mAllSources)
    {
        mSourcesPool.destroy(currSource);
    }
    mAllSources.clear();

    // destroy buffers
    for (AudioBuffer* currBuffer: mAllBuffers)
    {
        mBuffersPool.destroy(currBuffer);
    }
    mAllBuffers.clear();

    // destroy context
    if (mContext)
    {
        ::alcMakeContextCurrent(nullptr);
        ::alcDestroyContext(mContext);
        mContext = nullptr;
    }

    // shutdown device
    if (mDevice)
    {
        ::alcCloseDevice(mDevice);
        mDevice = nullptr;
    }
}

bool AudioDevice::IsInitialized() const
{
    return mDevice != nullptr;
}

bool AudioDevice::SetMasterVolume(float gainValue)
{
    if (IsInitialized())
    {
        ::alListenerf(AL_GAIN, gainValue);
        alCheckError();
        return true;
    }
    return false;
}

AudioBuffer* AudioDevice::CreateAudioBuffer()
{
    AudioBuffer* audioBuffer = mBuffersPool.create();
    debug_assert(audioBuffer);
    if (audioBuffer)
    {
        mAllBuffers.push_back(audioBuffer);
    }
    return audioBuffer;
}

AudioBuffer* AudioDevice::CreateAudioBuffer(int sampleRate, int bitsPerSample, int channelsCount, int dataLength, const void* bufferData)
{
    AudioBuffer* audioBuffer = CreateAudioBuffer();
    debug_assert(audioBuffer);

    if (audioBuffer)
    {
        if (!audioBuffer->SetupBufferData(sampleRate, bitsPerSample, channelsCount, dataLength, bufferData))
        {
            debug_assert(false);
        }
    }
    return audioBuffer;
}

void AudioDevice::DestroyAudioBuffer(AudioBuffer* audioBuffer)
{
    if (audioBuffer)
    {
        cxx::erase_elements(mAllBuffers, audioBuffer);
        mBuffersPool.destroy(audioBuffer);
    }
}

AudioSource* AudioDevice::CreateAudioSource()
{
    AudioSource* audioSource = mSourcesPool.create();
    debug_assert(audioSource);
    if (audioSource)
    {
        mAllSources.push_back(audioSource);
    }
    return audioSource;
}

void AudioDevice::DestroyAudioSource(AudioSource* audioSource)
{
    if (audioSource)
    {
        cxx::erase_elements(mAllSources, audioSource);
        mSourcesPool.destroy(audioSource);
    }
}

AudioListener* AudioDevice::CreateAudioListener()
{
    AudioListener* instance = new AudioListener;

    mAllListeners.push_back(instance);
    return instance;
}

void AudioDevice::DestroyAudioListener(AudioListener* audioListener)
{
    if (audioListener)
    {
        cxx::erase_elements(mAllListeners, audioListener);
        delete audioListener;
    }
}

void AudioDevice::UpdateFrame()
{
    if (IsInitialized())
    {
        UpdateSourcesPositions();
    }
}

void AudioDevice::UpdateSourcesPositions()
{
    for (AudioSource* currSource: mAllSources)
    {
        if (!currSource->IsPlaying())
            continue;

        const glm::vec3& sourceLocation = currSource->mSourceLocation;

        AudioListener* nearestListener = nullptr;
        float distanceToListener2 = 0.0f;
        // find the nearest listener
        for (AudioListener* currListener: mAllListeners)
        {
            if (nearestListener == nullptr)
            {
                nearestListener = currListener;
                distanceToListener2 = glm::distance2(sourceLocation, currListener->mPosition);
                continue;
            }
            float dist2 = glm::distance2(sourceLocation, currListener->mPosition);
            if (dist2 < distanceToListener2)
            {
                nearestListener = currListener;
                distanceToListener2 = dist2;
            }
        }

        glm::vec3 listenerLocation {0.0f, 0.0f, 1.0f};
        if (nearestListener)
        {
            listenerLocation = nearestListener->mPosition;
        }

        // relative position
        float posx = sourceLocation.x - listenerLocation.x;
        float posz = sourceLocation.z - listenerLocation.z;

        ::alSource3f(currSource->mSourceID, AL_POSITION, posx, sourceLocation.y, posz);
        alCheckError();
    }
}
