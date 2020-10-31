#pragma once

#include "CommonTypes.h"

// forwards
class Cvar;

// represents console system that handles debug commands
class Console final: public cxx::noncopyable
{
public:
    // readonly
    std::deque<ConsoleLine> mLines;
    std::vector<Cvar*> mCvarsList;

public:
    // Setup internal resources, returns false on error
    bool Initialize();
    void Deinit();
    void RegisterGlobalVariables();

    // Write text message in console
    void LogMessage(eLogMessage messageCat, const char* format, ...);

    // Clear all console text messages
    void Flush();

    // parse and execute commands
    // @param commands: Commands string
    void ExecuteCommands(const char* commands);

    // Register or unregister console variable
    // @returns false on error
    bool RegisterVariable(Cvar* consoleVariable);
    bool UnregisterVariable(Cvar* consoleVariable);
};

extern Console gConsole;