#pragma once

class TimeManager: public cxx::noncopyable
{
public:
    float mSystemTime = 0.0f;
    float mSystemFrameDelta = 0.0f;

    float mGameTime = 0.0f;
    float mGameFrameDelta = 0.0f;
    float mGameTimeScale = 1.0f;

    float mUiTime = 0.0f;
    float mUiFrameDelta = 0.0f;
    float mUiTimeScale = 1.0f;

    float mMinFramerate = 20.0f;
    float mMaxFramerate = 120.0f;

public:
    // Setup manager internal resources
    bool Initialize();
    void Deinit();

    void UpdateFrame();

    // Set fps limitations
    void SetMinFramerate(float framesPerSecond);
    void SetMaxFramerate(float framesPerSecond);

    // Scale game time, timeScale to 1.0 means no scale applied
    void SetGameTimeScale(float timeScale);
    void SetUiTimeScale(float timeScale);

private:
    double mMaxFrameDelta = 0.0f;
    double mMinFrameDelta = 0.0f;
    double mLastFrameTimestamp = 0.0f;
};

extern TimeManager gTimeManager;