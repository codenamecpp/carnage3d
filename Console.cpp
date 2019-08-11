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
    cxx::f_vsnprintf(ConsoleMessageBuffer, sizeof(ConsoleMessageBuffer), format, vaList);
    VA_SCOPE_CLOSE(vaList)

    if (messageCat > eLogMessage_Debug)
    {
        const char* logLevelString = ToString(messageCat);
        printf("%s: %s\n", logLevelString, ConsoleMessageBuffer);
    }
    else
    {
        printf("%s\n", ConsoleMessageBuffer);
    }
}

void Console::Flush()
{
}
