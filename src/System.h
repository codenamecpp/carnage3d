#pragma once

const int SysMemoryFrameHeapSize = 12 * 1024 * 1024;
const int DefaultScreenResolutionX = 1024;
const int DefaultScreenResolutionY = 768;

// defines system configuration
class SysConfig
{
public:
    SysConfig() = default;
    SysConfig(int screenSizex, int screenSizey, bool fullscreen, bool vsync);

    // set config default params
    void SetDefaultParams();

    // Set screen dimensions and automatically compute aspect ratio
    // @param screenSizex, screenSizey: Dimensions
    void SetScreenSize(int screenSizex, int screenSizey);

    // Set screen dimensions and automatically compute aspect ratio
    // @param screenSizex, screenSizey: Dimensions
    void SetParams(int screenSizex, int screenSizey, bool fullscreen, bool vsync);

public:
    // graphics settings
    int mScreenSizex = DefaultScreenResolutionX; // screen dimensions
    int mScreenSizey = DefaultScreenResolutionY; 
    bool mFullscreen = false; // enable full screen mode
    bool mEnableVSync = false; // enable vertical synchronization
    bool mOpenGLCoreProfile = true;
    float mScreenAspectRatio = 1.0f;
    // memory settings
    bool mEnableFrameHeapAllocator = true;
    // debug settings
    bool mShowImguiDemoWindow = false;
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
    cxx::string_buffer_256 mGtaDataLocation; // force gta data location
    int mPlayersCount = 0;
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
    void HandleEvent(GamepadInputEvent& inputEvent);

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