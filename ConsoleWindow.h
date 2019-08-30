#pragma once

#include "DebugWindow.h"

struct ImGuiInputTextCallbackData;

// defines debug console window as part of imgui system
class ConsoleWindow final: public DebugWindow
{
public:
    bool AutoScroll = true;
    bool ScrollToBottom = false;

public:
    ConsoleWindow();

    // process logic
    // @param deltaTime: Time passed since previous update
    void DoUI(Timespan deltaTime) override;

    void AddLog(const char* fmt, ...);
    void ExecCommand(const char* command_line);

private:
    int TextEditCallback(ImGuiInputTextCallbackData* data);

private:
    cxx::string_buffer_256 mInputBuffer;
    std::vector<std::string> mItems;
    std::vector<std::string> mCommands;
    std::vector<std::string> mHistory;
    int mHistoryPos = -1; // -1: new line, 0..History.Size-1 browsing history.
};

extern ConsoleWindow gDebugConsoleWindow;