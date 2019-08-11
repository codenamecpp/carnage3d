#include "stdafx.h"
#include "System.h"
#include "GraphicsDevice.h"
#include <mmsystem.h>

//////////////////////////////////////////////////////////////////////////

static const char* SysConfigPath = "config/sys_config.json";

//////////////////////////////////////////////////////////////////////////

System gSystem;

System::System() : mQuitRequested(), mStartTimestamp()
{
}

void System::Execute()
{
    Initialize();

    // map loop

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
        gCarnageGame.UpdateFrame(deltaTime);
        gGraphicsDevice.Present();
        mPreviousFrameTimestamp = mCurrentTimestamp;
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

    if (!gCarnageGame.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Canot initialize game application");
        Terminate();
    }
    mQuitRequested = false;
}

void System::Deinit()
{
    gConsole.LogMessage(eLogMessage_Info, "System shutdown");

    ::timeEndPeriod(1);

    gCarnageGame.Deinit();
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
    gInputs.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(MouseMovedInputEvent& inputEvent)
{
    gInputs.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(MouseScrollInputEvent& inputEvent)
{
    gInputs.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(KeyInputEvent& inputEvent)
{
    gInputs.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

void System::HandleEvent(KeyCharEvent& inputEvent)
{
    gInputs.HandleEvent(inputEvent);
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

    cxx::config_element screenConfig = configDocument.get_root_element().get_child("screen");
    if (!screenConfig)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Screen config section is missed");
        return false;
    }

    int screen_sizex = DefaultResolutionX;
    int screen_sizey = DefaultResolutionY;
    if (cxx::config_element screenResolution = screenConfig.get_child("resolution"))
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
    const char* gta_data_root = configDocument.get_root_element().get_child("gta_gamedata_location").get_value_string();
    gFiles.AddSearchPlace(gta_data_root);

    return true;
}

bool System::SaveConfiguration()
{
    // todo
    return true;
}
