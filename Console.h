#pragma once

#include "CommonTypes.h"

// represents console system that handles debug commands
class Console final: public cxx::noncopyable
{
public:
    // Setup internal resources, returns false on error
    bool Initialize();

    // Free allocated resources
    void Deinit();

    // Write text message in console, it could be ignored depending on currenyl active importance level filter
    // @param messageType: Message category
    // @param format: String format
    // @args: Arguments
    void LogMessage(eLogMessage messageCat, const char* format, ...);

    // Clear all console text messages
    void Flush();

public:
    std::deque<ConsoleLine> mLines;
};

extern Console gConsole;