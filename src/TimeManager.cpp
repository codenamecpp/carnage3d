#include "stdafx.h"
#include "TimeManager.h"

TimeManager gTimeManager;

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

    return true;
}

void TimeManager::Deinit()
{
}

void TimeManager::AdvanceTime(double deltaSeconds)
{
    debug_assert(deltaSeconds >= 0.0);
    mSystemFrameDelta = (float) std::max(deltaSeconds, 0.0);
    mSystemTime += mSystemFrameDelta;

    mGameFrameDelta = (float) (mGameTimeScale * deltaSeconds);
    mGameTime += mGameFrameDelta;
    
    mUiFrameDelta = (float) (mUiTimeScale * deltaSeconds);
    mUiTime += mUiFrameDelta;
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