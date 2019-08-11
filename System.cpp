#include "stdafx.h"
#include "System.h"
#include "GraphicsDevice.h"
#include <mmsystem.h>

//////////////////////////////////////////////////////////////////////////

static const char* settingsPath = "config/sys_config.json";

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
    mConfig.SetParams(DefaultResolutionX, DefaultResolutionY, false, false); // defaults

    gConsole.LogMessage(eLogMessage_Info, "Screen resolution: %dx%d, Vsync: %s", 
        mConfig.mScreenSizex, 
        mConfig.mScreenSizey, mConfig.mEnableVSync ? "enabled" : "disabled");

    // gta1 data files location

    return true;
}

bool System::SaveConfiguration()
{
    // todo
    return true;
}
