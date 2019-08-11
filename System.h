#pragma once

// defines system configuration
class SysConfig
{
public:
    SysConfig() = default;
    SysConfig(int screenSizex, int screenSizey, bool fullscreen, bool vsync)
        : mScreenSizex(screenSizex)
        , mScreenSizey(screenSizey)
        , mFullscreen(fullscreen)
        , mEnableVSync(vsync)
        , mScreenAspectRatio(1.0f)
    {
        mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
    }

    // Set screen dimensions and automatically compute aspect ratio
    // @param screenSizex, screenSizey: Dimensions
    void SetScreenSize(int screenSizex, int screenSizey)
    {
        mScreenSizex = screenSizex;
        mScreenSizey = screenSizey;
        mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
    }

    // Set screen dimensions and automatically compute aspect ratio
    // @param screenSizex, screenSizey: Dimensions
    void SetParams(int screenSizex, int screenSizey, bool fullscreen, bool vsync)
    {
        mEnableVSync = vsync;
        mFullscreen = fullscreen;
        mScreenSizex = screenSizex;
        mScreenSizey = screenSizey;
        mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
    }

public:
    int mScreenSizex = 0; // screen dimensions
    int mScreenSizey = 0; 
    bool mFullscreen = false; // enable full screen mode
    bool mEnableVSync = false; // enable vertical synchronization
    float mScreenAspectRatio = 1.0f;
};

// Common system specific stuff collected in System class
class System final: public cxx::noncopyable
{
public:
    // public just for convenience
    SysConfig mConfig; // current configuration of the system

public:
    System();

    // Initialize game subsystems and run main loop
    void Execute();

    // Abnormal application shutdown due to critical failure
    void Terminate();

    // Process input event, all events will be propagated then to game subsystems
    // @param inputEvent: Event data
    void HandleEvent(MouseButtonInputEvent& inputEvent);
    void HandleEvent(MouseMovedInputEvent& inputEvent);
    void HandleEvent(MouseScrollInputEvent& inputEvent);
    void HandleEvent(KeyInputEvent& inputEvent);
    void HandleEvent(KeyCharEvent& inputEvent);

    // Set application exit request flag, execution will be interrupted soon
    void QuitRequest();

    // Get milliseconds since system started
    long GetSysMilliseconds() const;

private:
    void Initialize();
    void Deinit();

    // Save/Load configuration to/from external file
    bool LoadConfiguration();
    bool SaveConfiguration();

private:
    long mStartTimestamp;
    bool mQuitRequested;
};

extern System gSystem;