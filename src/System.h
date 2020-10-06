#pragma once

// defines system configuration
class SystemConfig
{
public:
    SystemConfig();

    // reset config to defaults
    void SetToDefaults();

    void InitFromJsonDocument(const cxx::json_document& sourceDocument);
    void ExportToJsonDocument(cxx::json_document& sourceDocument);

public:
    // graphics
    int mScreenSizex, mScreenSizey; // screen dimensions
    bool mFullscreen; // enable full screen mode
    bool mEnableVSync; // enable vertical synchronization

    // physics
    float mPhysicsFramerate;

    // memory
    bool mEnableFrameHeapAllocator;

    // audio
    bool mEnableAudio;

    // debug
    bool mShowImguiDemoWindow;
};

//////////////////////////////////////////////////////////////////////////

// defines system startup parameters
class SystemStartupParams
{
public:
    SystemStartupParams() = default;

    bool ParseStartupParams(int argc, char *argv[]);
    // clear all startup params
    void ClearParams();

public:
    std::string mDebugMapName; // startup map name
    std::string mGtaDataLocation; // force gta data location
    std::string mGtaGameVersion; // force gta version
    std::string mGameLanguage; // force ingame language
    int mPlayersCount = 0;
};

//////////////////////////////////////////////////////////////////////////

// Common system specific stuff collected in System class
class System final: public cxx::noncopyable
{
public:
    // public just for convenience
    SystemConfig mConfig; // current configuration of the system
    SystemStartupParams mStartupParams;

public:
    // Initialize game subsystems and run main loop
    void Run(int argc, char *argv[]);

    // Abnormal application shutdown due to critical failure
    void Terminate();

    // Set application exit request flag, execution will be interrupted soon
    void QuitRequest();

    // Get real time seconds since system started
    double GetSystemSeconds() const;

private:
    void Initialize(int argc, char *argv[]);
    void Deinit();
    bool ExecuteFrame();

    // Save/Load configuration to/from external file
    bool LoadConfiguration();
    bool SaveConfiguration();

private:
    bool mQuitRequested;
};

extern System gSystem;