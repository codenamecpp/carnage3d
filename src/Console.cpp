#include "stdafx.h"
#include "Console.h"

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
    cxx::string_tokenizer tokenizer(commands);
    for (;;)
    {
        std::string commandName;
        if (!tokenizer.get_next(commandName, ' '))
            break;

        std::string commandParams;
        if (tokenizer.get_next(commandParams, ';'))
        {
            cxx::trim(commandParams);
        }

        cxx::trim(commandName);

        // todo:
        // vars
        // commands
        LogMessage(eLogMessage_Warning, "Unknown command %s", commandName.c_str());
    }
}
