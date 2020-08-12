#include "stdafx.h"
#include "TimeManager.h"

TimeManager gTimeManager;

//////////////////////////////////////////////////////////////////////////

inline void SetupMultimediaTimers()
{
#if OS_NAME == OS_WINDOWS
    MMRESULT mmResult = ::timeBeginPeriod(1);
    if (mmResult != TIMERR_NOERROR)
    {
        gConsole.LogMessage(eLogMessage_Debug, "Cannot setup multimedia timers");
    }
#endif
}

inline void ShutdownMultimediaTimers()
{
#if OS_NAME == OS_WINDOWS
    ::timeEndPeriod(1); 
#endif
}

//////////////////////////////////////////////////////////////////////////

bool TimeManager::Initialize()
{
    mSystemTime = 0.0f;
    mSystemFrameDelta = 0.0f;

    mGameTime = 0.0f;
    mGameFrameDelta = 0.0f;
    mGameTimeScale = 1.0f;

    mUiTime = 0.0f;
    mUiFrameDelta = 0.0f;
    mUiTimeScale = 1.0f;

    mMaxFrameDelta = 0.0;
    mMinFrameDelta = 0.0;

    // setup default frame limits
    SetMaxFramerate(120.0f);
    SetMinFramerate(20.0f);

    SetupMultimediaTimers();

    mLastFrameTimestamp = gSystem.GetSystemSeconds();

    return true;
}

void TimeManager::Deinit()
{
    ShutdownMultimediaTimers();
}

void TimeManager::UpdateFrame()
{
    double currentTimestamp = gSystem.GetSystemSeconds();
    double systemDeltaTime = (currentTimestamp - mLastFrameTimestamp);
    // limit fps 
    while (systemDeltaTime < mMinFrameDelta)
    {
        std::this_thread::sleep_for(std::chrono::seconds(0));
        currentTimestamp = gSystem.GetSystemSeconds();
        systemDeltaTime = (currentTimestamp - mLastFrameTimestamp);
    }

    if (systemDeltaTime > mMaxFrameDelta)
    {
        systemDeltaTime = mMaxFrameDelta;
    }

    // update timers
    debug_assert(systemDeltaTime >= 0.0);
    mSystemFrameDelta = (float) std::max(systemDeltaTime, 0.0);
    mSystemTime += mSystemFrameDelta;

    mGameFrameDelta = (float) (mGameTimeScale * systemDeltaTime);
    mGameTime += mGameFrameDelta;
    
    mUiFrameDelta = (float) (mUiTimeScale * systemDeltaTime);
    mUiTime += mUiFrameDelta;

    mLastFrameTimestamp = currentTimestamp;
}

void TimeManager::SetGameTimeScale(float timeScale)
{
    debug_assert(timeScale >= 0.0f);
    mGameTimeScale = std::max(timeScale, 0.0f);
}

void TimeManager::SetUiTimeScale(float timeScale)
{
    debug_assert(timeScale >= 0.0f);
    mUiTimeScale = std::max(timeScale, 0.0f);
}

void TimeManager::SetMinFramerate(float framesPerSecond)
{
    debug_assert(framesPerSecond >= 0.0f);
    mMinFramerate = std::max(framesPerSecond, 1.0f);
    mMaxFrameDelta = 1.0 / mMinFramerate;
}

void TimeManager::SetMaxFramerate(float framesPerSecond)
{
    debug_assert(framesPerSecond >= 0.0f);
    mMaxFramerate = std::max(framesPerSecond, 1.0f);
    mMinFrameDelta = 1.0 / mMaxFramerate;
}