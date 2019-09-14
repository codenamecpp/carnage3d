#include "stdafx.h"
#include "System.h"
#include "GraphicsDevice.h"
#include "RenderingManager.h"
#include <mmsystem.h>

//////////////////////////////////////////////////////////////////////////

static const char* SysConfigPath = "config/sys_config.json";

//////////////////////////////////////////////////////////////////////////

SysConfig::SysConfig(int screenSizex, int screenSizey, bool fullscreen, bool vsync)
    : mScreenSizex(screenSizex)
    , mScreenSizey(screenSizey)
    , mFullscreen(fullscreen)
    , mEnableVSync(vsync)
    , mScreenAspectRatio(1.0f)
{
    mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
}

void SysConfig::SetScreenSize(int screenSizex, int screenSizey)
{
    mScreenSizex = screenSizex;
    mScreenSizey = screenSizey;
    mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
}

void SysConfig::SetParams(int screenSizex, int screenSizey, bool fullscreen, bool vsync)
{
    mEnableVSync = vsync;
    mFullscreen = fullscreen;
    mScreenSizex = screenSizex;
    mScreenSizey = screenSizey;
    mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
}

//////////////////////////////////////////////////////////////////////////

System gSystem;

void System::Execute()
{
    mIgnoreInputs = true; // don't dispatch input events until initialization completed

    Initialize();

    // main loop
    long mPreviousFrameTimestamp = GetSysMilliseconds();
    for (; !mQuitRequested; )
    {
        long mCurrentTimestamp = GetSysMilliseconds();
        if (mCurrentTimestamp - mPreviousFrameTimestamp < 1)
        {
            ::Sleep(1);
            mCurrentTimestamp = GetSysMilliseconds();
        }

        Timespan deltaTime ( mCurrentTimestamp - mPreviousFrameTimestamp );

        // order in which subsystems gets updated is significant
        gGuiSystem.UpdateFrame(deltaTime);
        gCarnageGame.UpdateFrame(deltaTime);
        gRenderManager.RenderFrame();
        mPreviousFrameTimestamp = mCurrentTimestamp;
        if (mIgnoreInputs) // ingore inputs at very first frame
        {
            mIgnoreInputs = false;
        }
    }

    Deinit();
}

void System::Initialize()
{
    if (!gConsole.Initialize())
    {
        debug_assert(false);
    }

    gConsole.LogMessage(eLogMessage_Info, "System initialize");
    
    // startup multimedia timer
    MMRESULT mmResult = ::timeBeginPeriod(1);
    if (mmResult != TIMERR_NOERROR)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Could not initialize multimedia timer");
    }

    mStartTimestamp = ::timeGetTime();

    if (!gFiles.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize filesystem");
        Terminate();
    }

    if (!LoadConfiguration())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot load configuration");
        Terminate();
    }

    if (!gGraphicsDevice.Initialize(mConfig.mScreenSizex, mConfig.mScreenSizey, mConfig.mFullscreen, mConfig.mEnableVSync))
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize graphics device");
        Terminate();
    }

    if (!gRenderManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize render system");
        Terminate();
    }

    if (!gGuiSystem.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize gui system");
        Terminate();
    }

    if (!gCarnageGame.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize game");
        Terminate();
    }
    mQuitRequested = false;
}

void System::Deinit()
{
    gConsole.LogMessage(eLogMessage_Info, "System shutdown");

    ::timeEndPeriod(1);

    gCarnageGame.Deinit();
    gGuiSystem.Deinit();
    gRenderManager.Deinit();
    gGraphicsDevice.Deinit();
    gFiles.Deinit();
    gConsole.Deinit();
}

void System::Terminate()
{    
    Deinit(); // leave gracefully
    exit(EXIT_FAILURE);
}

void System::QuitRequest()
{
    mQuitRequested = true;
}

void System::HandleEvent(MouseButtonInputEvent& inputEvent)
{
    if (mIgnoreInputs)
        return;

    gInputs.HandleEvent(inputEvent);
    gGuiSystem.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(MouseMovedInputEvent& inputEvent)
{
    if (mIgnoreInputs)
        return;

    gInputs.HandleEvent(inputEvent);
    gGuiSystem.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(MouseScrollInputEvent& inputEvent)
{
    if (mIgnoreInputs)
        return;

    gInputs.HandleEvent(inputEvent);
    gGuiSystem.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(KeyInputEvent& inputEvent)
{
    if (mIgnoreInputs)
        return;

    gInputs.HandleEvent(inputEvent);
    gGuiSystem.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(KeyCharEvent& inputEvent)
{
    if (mIgnoreInputs)
        return;

    gInputs.HandleEvent(inputEvent);
    gGuiSystem.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

long System::GetSysMilliseconds() const
{
    const unsigned long totalMilliseconds = ::timeGetTime();
    return (long) totalMilliseconds - mStartTimestamp;
}

bool System::LoadConfiguration()
{
    const int DefaultResolutionX = 1280;
    const int DefaultResolutionY = 900;

    // read config
    std::string jsonContent;
    if (!gFiles.ReadTextFile(SysConfigPath, jsonContent))
        return false;

    cxx::config_document configDocument;
    if (!configDocument.parse_document(jsonContent.c_str()))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse configuration file");
        return false;
    }

    cxx::config_node screenConfig = configDocument.get_root_node().get_child("screen");
    if (!screenConfig)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Screen config section is missed");
        return false;
    }

    int screen_sizex = DefaultResolutionX;
    int screen_sizey = DefaultResolutionY;
    if (cxx::config_node screenResolution = screenConfig.get_child("resolution"))
    {
        screen_sizex = screenResolution.get_array_element(0).get_value_integer();
        screen_sizey = screenResolution.get_array_element(1).get_value_integer();
    }

    bool fullscreen_mode = screenConfig.get_child("fullscreen").get_value_boolean();
    bool vsync_mode = screenConfig.get_child("vsync").get_value_boolean();
    bool hardware_cursor = screenConfig.get_child("hardware_cursor").get_value_boolean();

    mConfig.SetParams(screen_sizex, screen_sizey, fullscreen_mode, vsync_mode);

    gConsole.LogMessage(eLogMessage_Info, "Screen resolution: %dx%d, Vsync: %s", 
        mConfig.mScreenSizex, 
        mConfig.mScreenSizey, mConfig.mEnableVSync ? "enabled" : "disabled");

    // gta1 data files location
    const char* gta_data_root = configDocument.get_root_node().get_child("gta_gamedata_location").get_value_string();
    gFiles.AddSearchPlace(gta_data_root);

    return true;
}

bool System::SaveConfiguration()
{
    // todo
    return true;
}
