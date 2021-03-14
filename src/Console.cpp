#include "stdafx.h"
#include "Console.h"
#include "ConsoleVar.h"
#include "cvars.h"

static char ConsoleMessageBuffer[2048];

#define VA_SCOPE_OPEN(firstArg, vaName) \
    { \
        va_list vaName {}; \
        va_start(vaName, firstArg); \

#define VA_SCOPE_CLOSE(vaName) \
        va_end(vaName); \
    }

Console gConsole;

bool Console::Initialize()
{
    return true;
}

void Console::Deinit()
{
}

void Console::LogMessage(eLogMessage messageCat, const char* format, ...)
{
    VA_SCOPE_OPEN(format, vaList)
    vsnprintf(ConsoleMessageBuffer, sizeof(ConsoleMessageBuffer), format, vaList);
    VA_SCOPE_CLOSE(vaList)

    if (messageCat > eLogMessage_Debug)
    {
        printf("%s\n", ConsoleMessageBuffer);
    }
    ConsoleLine consoleLine;
    consoleLine.mLineType = eConsoleLineType_Message;
    consoleLine.mMessageCategory = messageCat;
    consoleLine.mString = ConsoleMessageBuffer;
    mLines.push_back(std::move(consoleLine));
}

void Console::Flush()
{
    mLines.clear();
}

void Console::ExecuteCommands(const char* commands)
{
    LogMessage(eLogMessage_Debug, "%s", commands); // echo

    cxx::string_tokenizer tokenizer(commands);
    for (;;)
    {
        std::string commandName;
        if (!tokenizer.get_next(commandName, ' '))
            break;

        // find variable by name
        Cvar* consoleVariable = nullptr;
        for (Cvar* currCvar: mCvarsList)
        {
            if (currCvar->mName == commandName)
            {
                consoleVariable = currCvar;
                break;
            }
        }

        if (consoleVariable)
        {   
            std::string commandParams;
            if (tokenizer.get_next(commandParams, ';'))
            {
                cxx::trim(commandParams);
            }
            consoleVariable->CallWithParams(commandParams);
            break;
        }
        LogMessage(eLogMessage_Warning, "Unknown command %s", commandName.c_str());
    }
}

bool Console::RegisterVariable(Cvar* consoleVariable)
{
    if (consoleVariable == nullptr)
    {
        debug_assert(false);
        return false;
    }
    if (cxx::contains_if(mCvarsList, [consoleVariable](const Cvar* currCvar)
        {
            return (currCvar == consoleVariable) || (currCvar->mName == consoleVariable->mName);
        }))
    {
        debug_assert(false);
        return false;
    }
    mCvarsList.push_back(consoleVariable);
    return true;
}

bool Console::UnregisterVariable(Cvar* consoleVariable)
{
    if (consoleVariable == nullptr)
    {
        debug_assert(false);
        return false;
    }
    cxx::erase_elements(mCvarsList, consoleVariable);
    return true;
}

void Console::RegisterGlobalVariables()
{
    CvarsRegisterGlobal();
}
