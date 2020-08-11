#include "stdafx.h"
#include "System.h"
#include "GraphicsDevice.h"
#include "RenderingManager.h"
#include "MemoryManager.h"
#include "CarnageGame.h"
#include "ImGuiManager.h"
#include "TimeManager.h"

//////////////////////////////////////////////////////////////////////////

static const char* SysConfigPath = "config/sys_config.json";

//////////////////////////////////////////////////////////////////////////

SystemConfig::SystemConfig(int screenSizex, int screenSizey, bool fullscreen, bool vsync)
    : mScreenSizex(screenSizex)
    , mScreenSizey(screenSizey)
    , mFullscreen(fullscreen)
    , mEnableVSync(vsync)
    , mScreenAspectRatio(1.0f)
{
    mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
}

void SystemConfig::SetDefaultParams()
{
    mEnableFrameHeapAllocator = true;
    mShowImguiDemoWindow = false;

    SetParams(DefaultScreenResolutionX, DefaultScreenResolutionY, false, false);
}

void SystemConfig::SetScreenSize(int screenSizex, int screenSizey)
{
    mScreenSizex = screenSizex;
    mScreenSizey = screenSizey;
    mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
}

void SystemConfig::SetParams(int screenSizex, int screenSizey, bool fullscreen, bool vsync)
{
    mEnableVSync = vsync;
    mFullscreen = fullscreen;
    mScreenSizex = screenSizex;
    mScreenSizey = screenSizey;
    mScreenAspectRatio = (mScreenSizey > 0) ? ((mScreenSizex * 1.0f) / (mScreenSizey * 1.0f)) : 1.0f;
}

//////////////////////////////////////////////////////////////////////////

bool SystemStartupParams::ParseStartupParams(int argc, char *argv[])
{
    ClearParams();

    for (int iarg = 1; iarg < argc; )
    {
        if (cxx_stricmp(argv[iarg], "-mapname") == 0 && (argc > iarg + 1))
        {
            mDebugMapName.assign(argv[iarg + 1]);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-gtadata") == 0 && (argc > iarg + 1))
        {
            mGtaDataLocation.assign(argv[iarg + 1]);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-numplayers") == 0 && (argc > iarg + 1))
        {
            ::sscanf(argv[iarg + 1], "%d", &mPlayersCount);
            iarg += 2;
            continue;
        }
        ++iarg;
    }

    return true;
}

void SystemStartupParams::ClearParams()
{
    mDebugMapName.clear();
    mGtaDataLocation.clear();
    mPlayersCount = 0;
}

//////////////////////////////////////////////////////////////////////////

System gSystem;

void System::Initialize(int argc, char *argv[])
{
    if (!gConsole.Initialize())
    {
        debug_assert(false);
    }

    gConsole.LogMessage(eLogMessage_Info, GAME_TITLE);
    gConsole.LogMessage(eLogMessage_Info, "System initialize");
    
    if (!mStartupParams.ParseStartupParams(argc, argv))
    {
        debug_assert(false);
    }

    if (!gFiles.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize filesystem");
        Terminate();
    }

    LoadConfiguration();

    if (!gFiles.SetupGtaDataLocation())
    {
        gConsole.LogMessage(eLogMessage_Error, "Set valid gta gamedata location via sys config param 'gta_gamedata_location'");
        Terminate();
    }

    if (!gMemoryManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize system memory manager");
        Terminate();
    }

    if (!gGraphicsDevice.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize graphics device");
        Terminate();
    }

    mStartSystemTime = ::glfwGetTime();
    debug_assert(mStartSystemTime > 0.0);

    if (!gImGuiManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize debug ui system");
        // ignore failure
    }

    if (!gRenderManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize render system");
        Terminate();
    }

    if (!gUiManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize gui system");
        Terminate();
    }

    if (!gCarnageGame.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize game");
        Terminate();
    }

    gTimeManager.Initialize();

    mQuitRequested = false;
}

void System::Deinit()
{
    gConsole.LogMessage(eLogMessage_Info, "System shutdown");

    gTimeManager.Deinit();
    gCarnageGame.Deinit();
    gImGuiManager.Deinit();
    gUiManager.Deinit();
    gRenderManager.Deinit();
    gGraphicsDevice.Deinit();
    gMemoryManager.Deinit();
    gFiles.Deinit();
    gConsole.Deinit();
}

void System::Execute()
{
    const double MinFPS = 20.0;
    const double MaxFrameDelta = 1.0 / MinFPS;

    const double MaxFPS = 120.0f;
    const double MinFrameDelta = 1.0 / MaxFPS;

    // main loop
    double previousFrameTimestamp = GetSystemSeconds();
    for (; !mQuitRequested; )
    {
        double currentTimestamp = GetSystemSeconds();
        double systemDeltaTime = (currentTimestamp - previousFrameTimestamp);
        if (systemDeltaTime < MinFrameDelta)
        {
            // limit fps 
            std::chrono::duration<double> sleepTime (MinFrameDelta - systemDeltaTime);
            std::this_thread::sleep_for(sleepTime);

            currentTimestamp = GetSystemSeconds();
            systemDeltaTime = (currentTimestamp - previousFrameTimestamp);
        }

        if (systemDeltaTime > MaxFrameDelta)
        {
            systemDeltaTime = MaxFrameDelta;
        }

        gTimeManager.AdvanceTime(systemDeltaTime);

        gMemoryManager.FlushFrameHeapMemory();
        // order in which subsystems gets updated is significant
        gImGuiManager.UpdateFrame();
        gUiManager.UpdateFrame();
        gCarnageGame.UpdateFrame();
        gRenderManager.RenderFrame();
        previousFrameTimestamp = currentTimestamp;
    }
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

bool System::LoadConfiguration()
{
    mConfig.SetDefaultParams();

    // read config
    std::string jsonContent;
    if (!gFiles.ReadTextFile(SysConfigPath, jsonContent))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load config '%s'", SysConfigPath);
        return false;
    }

    cxx::json_document configDocument;
    if (!configDocument.parse_document(jsonContent.c_str()))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse config '%s'", SysConfigPath);
        return false;
    }

    cxx::json_document_node configRootNode = configDocument.get_root_node();
    if (cxx::json_document_node screenConfig = configRootNode["screen"])
    {
        int screen_sizex = DefaultScreenResolutionX;
        int screen_sizey = DefaultScreenResolutionY;
        if (cxx::json_document_node screenResolution = screenConfig["resolution"])
        {
            cxx::json_get_attribute(screenResolution, 0, screen_sizex);
            cxx::json_get_attribute(screenResolution, 1, screen_sizey);
        }

        bool fullscreen_mode = mConfig.mFullscreen; 
        cxx::json_get_attribute(screenConfig, "fullscreen", fullscreen_mode);

        bool vsync_mode = mConfig.mEnableVSync;
        cxx::json_get_attribute(screenConfig, "vsync", vsync_mode);

        bool hardware_cursor = false;
        cxx::json_get_attribute(screenConfig, "hardware_cursor", hardware_cursor);

        mConfig.SetParams(screen_sizex, screen_sizey, fullscreen_mode, vsync_mode);
    }

    // gta1 data files location
    cxx::json_get_attribute(configRootNode, "gta_gamedata_location", gFiles.mGTADataDirectoryPath);

    // memory
    if (cxx::json_document_node memConfig = configRootNode["memory"])
    {
        cxx::json_get_attribute(memConfig, "enable_frame_heap_allocator", mConfig.mEnableFrameHeapAllocator);
    }

    // debug
    if (cxx::json_document_node memConfig = configRootNode["debug"])
    {
        cxx::json_get_attribute(memConfig, "show_imgui_demo_window", mConfig.mShowImguiDemoWindow);
    }
    return true;
}

bool System::SaveConfiguration()
{
    // todo
    return true;
}

double System::GetSystemSeconds() const
{   
    double currentTime = ::glfwGetTime();
    return (currentTime - mStartSystemTime);
}
