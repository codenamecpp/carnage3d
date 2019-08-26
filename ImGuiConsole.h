#pragma once

#include "imgui.h"

// defines debug console window as part of imgui system
class ImGuiConsole final: public cxx::noncopyable
{
public:
    bool mShown = false;
    bool AutoScroll = true;
    bool ScrollToBottom = false;

public:
    ImGuiConsole();
    ~ImGuiConsole();

    // process logic
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(Timespan deltaTime);

    void ClearLog();
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