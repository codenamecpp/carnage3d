#pragma once

#include "Console.h"
#include "ConsoleVar.h"
#include "GameDefs.h"

// all console variables declarations

//////////////////////////////////////////////////////////////////////////

// graphics
extern CvarPoint gCvarGraphicsScreenDims; // screen dimensions
extern CvarBoolean gCvarGraphicsFullscreen; // is fullscreen mode enabled
extern CvarBoolean gCvarGraphicsVSync; // is vertical synchronization enabled
extern CvarBoolean gCvarGraphicsTexFiltering; // is texture filtering enabled

// physics
extern CvarFloat gCvarPhysicsFramerate; // physical world update framerate

// memory
extern CvarBoolean gCvarMemEnableFrameHeapAllocator; // enable frame heap allocator

// audio
extern CvarBoolean gCvarAudioActive; // enable audio system

// game
extern CvarString gCvarGtaDataPath; // config gta data location
extern CvarString gCvarMapname; // current map name
extern CvarString gCvarCurrentBaseDir; // current gta data location
extern CvarEnum<eGtaGameVersion> gCvarGameVersion; // current gta game version
extern CvarString gCvarGameLanguage; // current game language
extern CvarInt gCvarNumPlayers; // number of players in split screen mode
extern CvarBoolean gCvarWeatherActive; // whether weather effects enabled
extern CvarEnum<eWeatherEffect> gCvarWeatherEffect; // currently active weather

//////////////////////////////////////////////////////////////////////////

// console commands

extern CvarCommand gConSysQuit; // quit application
extern CvarCommand gConSysListCvars; // print all non-hidden cvars to console

//////////////////////////////////////////////////////////////////////////

inline void CvarsRegisterGlobal()
{
    // vars
    gConsole.RegisterVariable(&gCvarGraphicsScreenDims);
    gConsole.RegisterVariable(&gCvarGraphicsFullscreen);
    gConsole.RegisterVariable(&gCvarGraphicsVSync);
    gConsole.RegisterVariable(&gCvarGraphicsTexFiltering);
    gConsole.RegisterVariable(&gCvarPhysicsFramerate);
    gConsole.RegisterVariable(&gCvarMemEnableFrameHeapAllocator);
    gConsole.RegisterVariable(&gCvarAudioActive);
    gConsole.RegisterVariable(&gCvarGtaDataPath);
    gConsole.RegisterVariable(&gCvarMapname);
    gConsole.RegisterVariable(&gCvarCurrentBaseDir);
    gConsole.RegisterVariable(&gCvarGameVersion);
    gConsole.RegisterVariable(&gCvarGameLanguage);
    gConsole.RegisterVariable(&gCvarNumPlayers);
    gConsole.RegisterVariable(&gCvarWeatherActive);
    gConsole.RegisterVariable(&gCvarWeatherEffect);
    // commands
    gConsole.RegisterVariable(&gConSysQuit);
    gConsole.RegisterVariable(&gConSysListCvars);
}