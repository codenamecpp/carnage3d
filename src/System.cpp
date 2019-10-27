#include "stdafx.h"
#include "System.h"
#include "GraphicsDevice.h"
#include "RenderingManager.h"
#include "MemoryManager.h"
#include "CarnageGame.h"

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

void SysStartupParameters::SetNull()
{
    mDebugMapName.clear();
    mGtaDataLocation.clear();
}

//////////////////////////////////////////////////////////////////////////

System gSystem;

void System::Execute(const SysStartupParameters& sysStartupParams)
{
    const long MinFPS = 20;
    const long MaxFrameDelta = Timespan::MillisecondsPerSecond / MinFPS;

    mIgnoreInputs = true; // don't dispatch input events until initialization completed
    mStartupParams = sysStartupParams;
    Initialize();

    // main loop
    long previousFrameTimestamp = GetSysMilliseconds();
    for (; !mQuitRequested; )
    {
        long currentTimestamp = GetSysMilliseconds();

        Timespan deltaTime ( currentTimestamp - previousFrameTimestamp );
        if (deltaTime < 1)
        {
            // small delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            currentTimestamp = GetSysMilliseconds();
            deltaTime = 1;
        }

        if (deltaTime > MaxFrameDelta)
        {
            deltaTime = MaxFrameDelta;
        }

        gMemoryManager.FlushFrameHeapMemory();

        // order in which subsystems gets updated is significant
        gGuiSystem.UpdateFrame(deltaTime);
        gCarnageGame.UpdateFrame(deltaTime);
        gRenderManager.RenderFrame();
        previousFrameTimestamp = currentTimestamp;
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

    if (!gMemoryManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize system memory manager");
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

    gCarnageGame.Deinit();
    gGuiSystem.Deinit();
    gRenderManager.Deinit();
    gGraphicsDevice.Deinit();
    gMemoryManager.Deinit();
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

void System::HandleEvent(GamepadInputEvent& inputEvent)
{
    if (mIgnoreInputs)
        return;

    gInputs.HandleEvent(inputEvent);
    gCarnageGame.InputEvent(inputEvent);
}

long System::GetSysMilliseconds() const
{
    double totalSeconds = ::glfwGetTime();
    return static_cast<long>(totalSeconds * 1000.0);
}

bool System::LoadConfiguration()
{
    const int DefaultResolutionX = 1280;
    const int DefaultResolutionY = 900;

    // read config
    std::string jsonContent;
    if (!gFiles.ReadTextFile(SysConfigPath, jsonContent))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load config from '%s'", SysConfigPath);
        return false;
    }

    cxx::config_document configDocument;
    if (!configDocument.parse_document(jsonContent.c_str()))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse configuration document");
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

    // gta1 data files location
    const char* gta_data_root = configDocument.get_root_node().get_child("gta_gamedata_location").get_value_string();

    // override data location with startup param
    if (!mStartupParams.mGtaDataLocation.empty())
    {
        gta_data_root = mStartupParams.mGtaDataLocation.c_str();
    }

    if (*gta_data_root)
    {
        if (!cxx::is_directory_exists(gta_data_root))
        {
            gConsole.LogMessage(eLogMessage_Warning, "gta_gamedata_location directory does not exists");
        }
        else
        {
            gFiles.AddSearchPlace(gta_data_root);
            gConsole.LogMessage(eLogMessage_Info, "gta_gamedata_location: '%s'", gta_data_root);
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "gta_gamedata_location param is null");
    }

    // memory
    if (cxx::config_node memConfig = configDocument.get_root_node().get_child("memory"))
    {
        mConfig.mEnableFrameHeapAllocator = memConfig.get_child("enable_frame_heap_allocator").get_value_boolean();
    }

    // debug
    if (cxx::config_node memConfig = configDocument.get_root_node().get_child("debug"))
    {
        mConfig.mShowImguiDemoWindow = memConfig.get_child("show_imgui_demo_window").get_value_boolean();
    }
    return true;
}

bool System::SaveConfiguration()
{
    // todo
    return true;
}
