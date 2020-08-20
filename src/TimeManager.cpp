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
    double frameTimestamp = gSystem.GetSystemSeconds();
    double frameDelta = (frameTimestamp - mLastFrameTimestamp);
    // limit fps 
    while (frameDelta < mMinFrameDelta)
    {
        std::this_thread::sleep_for(std::chrono::seconds(0));

        double subFrameTimestamp = gSystem.GetSystemSeconds();
        double subFrameDelta = (subFrameTimestamp - frameTimestamp);

        frameDelta += subFrameDelta;
        frameTimestamp = subFrameTimestamp;
    }

    if (frameDelta > mMaxFrameDelta)
    {
        frameDelta = mMaxFrameDelta;
    }

    if (frameDelta < 0.0f)
    {
        debug_assert(false);
        frameDelta = 0.0f;
    }

    // update timers
    mSystemFrameDelta = (float) frameDelta;
    mSystemTime += mSystemFrameDelta;

    mGameFrameDelta = (float) (mGameTimeScale * frameDelta);
    mGameTime += mGameFrameDelta;
    
    mUiFrameDelta = (float) (mUiTimeScale * frameDelta);
    mUiTime += mUiFrameDelta;

    mLastFrameTimestamp = frameTimestamp;
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