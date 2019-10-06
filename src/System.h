#pragma once

// defines system configuration
class SysConfig
{
public:
    SysConfig() = default;
    SysConfig(int screenSizex, int screenSizey, bool fullscreen, bool vsync);

    // Set screen dimensions and automatically compute aspect ratio
    // @param screenSizex, screenSizey: Dimensions
    void SetScreenSize(int screenSizex, int screenSizey);

    // Set screen dimensions and automatically compute aspect ratio
    // @param screenSizex, screenSizey: Dimensions
    void SetParams(int screenSizex, int screenSizey, bool fullscreen, bool vsync);

public:
    int mScreenSizex = 0; // screen dimensions
    int mScreenSizey = 0; 
    bool mFullscreen = false; // enable full screen mode
    bool mEnableVSync = false; // enable vertical synchronization
    bool mOpenGLCoreProfile = true;
    float mScreenAspectRatio = 1.0f;
};

// defines system startup parameters
class SysStartupParameters
{
public:
    SysStartupParameters() = default;

    // clear all startup params
    void SetNull();

public:
    cxx::string_buffer_16 mDebugMapName; // startup map name
};

// Common system specific stuff collected in System class
class System final: public cxx::noncopyable
{
public:
    // public just for convenience
    SysConfig mConfig; // current configuration of the system
    SysStartupParameters mStartupParams;

public:
    // Initialize game subsystems and run main loop
    void Execute(const SysStartupParameters& sysStartupParams);

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
    bool mQuitRequested;
    bool mIgnoreInputs;
};

extern System gSystem;