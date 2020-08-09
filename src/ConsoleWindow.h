#pragma once

#include "DebugWindow.h"

struct ImGuiInputTextCallbackData;

// defines debug console window as part of imgui system
class ConsoleWindow final: public DebugWindow
{
public:
    bool mAutoScroll = true;
    bool mScrollToBottom = false;

public:
    ConsoleWindow();

    // process logic
    // @param deltaTime: Time passed since previous update
    void DoUI(ImGuiIO& imguiContext) override;

private:
    // internals
    int TextEditCallback(ImGuiInputTextCallbackData* data);
    void Exec();
    void MoveInputToHistory();

private:
    std::string mInputString;
    std::deque<std::string> mHistory;
    int mHistoryPos = -1; // -1: new line, 0..History.Size-1 browsing history.
};

extern ConsoleWindow gDebugConsoleWindow;