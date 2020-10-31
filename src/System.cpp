#include "stdafx.h"
#include "System.h"
#include "GraphicsDevice.h"
#include "RenderingManager.h"
#include "MemoryManager.h"
#include "CarnageGame.h"
#include "ImGuiManager.h"
#include "TimeManager.h"
#include "AudioDevice.h"
#include "AudioManager.h"
#include "cvars.h"

//////////////////////////////////////////////////////////////////////////

static const char* SysConfigPath = "config/sys_config.json";

//////////////////////////////////////////////////////////////////////////
// cvars
//////////////////////////////////////////////////////////////////////////

// graphics
CvarPoint gCvarGraphicsScreenDims("r_screenDims", Point(1024, 768), "Screen dimensions", CvarFlags_Archive | CvarFlags_RequiresAppRestart);
CvarBoolean gCvarGraphicsFullscreen("r_fullscreen", false, "Is fullscreen mode enabled", CvarFlags_Archive);
CvarBoolean gCvarGraphicsVSync("r_vsync", true, "Is vertical synchronization enabled", CvarFlags_Archive);
CvarBoolean gCvarGraphicsTexFiltering("r_texFiltering", false, "Is texture filtering enabled", CvarFlags_Archive | CvarFlags_Readonly);

// physics
CvarFloat gCvarPhysicsFramerate("g_physicsFps", 60.0f, "Physical world update framerate", CvarFlags_Archive | CvarFlags_Init);

// memory
CvarBoolean gCvarMemEnableFrameHeapAllocator("mem_enableFrameHeapAllocator", true, "Enable frame heap allocator", CvarFlags_Archive | CvarFlags_Init);

// audio
CvarBoolean gCvarAudioActive("a_audioActive", true, "Enable audio system", CvarFlags_Archive | CvarFlags_Init);

//////////////////////////////////////////////////////////////////////////

System gSystem;

void System::Initialize(int argc, char *argv[])
{
    mQuitRequested = false;

    if (!gConsole.Initialize())
    {
        debug_assert(false);
    }

    gConsole.LogMessage(eLogMessage_Info, GAME_TITLE);
    gConsole.LogMessage(eLogMessage_Info, "System initialize");
    gConsole.RegisterGlobalVariables();
    
    if (!gFiles.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize filesystem");
        Terminate();
    }

    LoadConfiguration();
    ParseStartupParams(argc, argv);

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

    if (gCvarAudioActive.mValue)
    {
        if (!gAudioDevice.Initialize())
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize audio device");
        }

        if (!gAudioManager.Initialize())
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize audio manager");
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Info, "Audio is disabled via config");
    }

    if (!gGuiManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize gui system");
        Terminate();
    }

    gTimeManager.Initialize();

    if (!gFiles.SetupGtaDataLocation())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Set valid gta gamedata location via sys config param 'g_gtadata'");
    }

    if (!gCarnageGame.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize game");
        Terminate();
    }
}

void System::Deinit(bool isTermination)
{
    gConsole.LogMessage(eLogMessage_Info, "System shutdown");

    if (!isTermination)
    {
        SaveConfiguration();
    }

    gTimeManager.Deinit();
    gCarnageGame.Deinit();
    gImGuiManager.Deinit();
    gGuiManager.Deinit();
    if (gAudioDevice.IsInitialized())
    {
        gAudioManager.Deinit();
        gAudioDevice.Deinit();
    }
    gRenderManager.Deinit();
    gGraphicsDevice.Deinit();
    gMemoryManager.Deinit();
    gFiles.Deinit();
    gConsole.Deinit();
}

void System::Run(int argc, char *argv[])
{
    Initialize(argc, argv);

    // main loop

#ifndef __EMSCRIPTEN__

    while (true)
    {
        bool continueExecution = ExecuteFrame();
        if (!continueExecution)
            break;
    }

    Deinit(false);

#else
    emscripten_set_main_loop([]()
    {
        bool continueExecution = gSystem.ExecuteFrame();
        if (!continueExecution)
        {
            gSystem.Deinit(false);
        }
    }, 
    0, false);
#endif // __EMSCRIPTEN__
}

void System::Terminate()
{    
    Deinit(true); // leave gracefully

#ifdef __EMSCRIPTEN__
    emscripten_force_exit(EXIT_FAILURE);
#else
    exit(EXIT_FAILURE);
#endif // __EMSCRIPTEN__
}

void System::QuitRequest()
{
    mQuitRequested = true;
}

bool System::LoadConfiguration()
{
    cxx::json_document configDocument;
    if (!gFiles.ReadConfig(SysConfigPath, configDocument))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load config from '%s'", SysConfigPath);
        return false;
    }

    cxx::json_document_node configRootNode = configDocument.get_root_node();

    // load archive cvars
    for (Cvar* currCvar: gConsole.mCvarsList)
    {
        if (!currCvar->IsArchive())
            continue;

        currCvar->LoadCvar(configRootNode);
    }

    return true;
}

bool System::SaveConfiguration()
{
    cxx::json_document configDocument;
    configDocument.create_document();

    cxx::json_document_node configRootNode = configDocument.get_root_node();

    // save archive cvars
    for (Cvar* currCvar: gConsole.mCvarsList)
    {
        if (!currCvar->IsArchive())
            continue;

        currCvar->SaveCvar(configRootNode);
    }

    std::ofstream outputFile;
    if (!gFiles.CreateTextFile(SysConfigPath, outputFile))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot save config to '%s'", SysConfigPath);
        return false;
    }

    std::string documentContent;
    configDocument.dump_document(documentContent);
    outputFile << documentContent;

    return true;
}

double System::GetSystemSeconds() const
{
    double currentTime = ::glfwGetTime();
    return currentTime;
}

bool System::ExecuteFrame()
{
    if (mQuitRequested)
        return false;

    gInputs.UpdateFrame();
    gTimeManager.UpdateFrame();
    gMemoryManager.FlushFrameHeapMemory();
    gImGuiManager.UpdateFrame();
    gGuiManager.UpdateFrame();
    gCarnageGame.UpdateFrame();
    if (gAudioDevice.IsInitialized())
    {
        gAudioManager.UpdateFrame();
        gAudioDevice.UpdateFrame(); // update at logic frame end
    }

    // update screen params
    if (gCvarGraphicsFullscreen.IsModified() || gCvarGraphicsVSync.IsModified())
    {
        gGraphicsDevice.EnableFullscreen(gCvarGraphicsFullscreen.mValue);
        gCvarGraphicsFullscreen.ClearModified();

        gGraphicsDevice.EnableVSync(gCvarGraphicsVSync.mValue);
        gCvarGraphicsVSync.ClearModified();
    }
    gRenderManager.RenderFrame();
    return true;
}

void System::ParseStartupParams(int argc, char *argv[])
{
    for (int iarg = 0; iarg < argc; )
    {
        if (cxx_stricmp(argv[iarg], "-mapname") == 0 && (argc > iarg + 1))
        {
            gCvarMapname.SetFromString(argv[iarg + 1], eCvarSetMethod_CommandLine);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-gtadata") == 0 && (argc > iarg + 1))
        {
            gCvarCurrentBaseDir.SetFromString(argv[iarg + 1], eCvarSetMethod_CommandLine);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-numplayers") == 0 && (argc > iarg + 1))
        {
            gCvarNumPlayers.SetFromString(argv[iarg + 1], eCvarSetMethod_CommandLine);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-gtaversion") == 0 && (argc > iarg + 1))
        {
            gCvarGameVersion.SetFromString(argv[iarg + 1], eCvarSetMethod_CommandLine);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-lang") == 0 && (argc > iarg + 1))
        {
            gCvarGameLanguage.SetFromString(argv[iarg + 1], eCvarSetMethod_CommandLine);
            iarg += 2;
            continue;
        }
        gConsole.LogMessage(eLogMessage_Warning, "Unknown arg '%s'", argv[iarg]);
        ++iarg;
    }
}