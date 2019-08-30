#include "stdafx.h"
#include "DebugWindow.h"
#include "ImGuiManager.h"

cxx::intrusive_list<DebugWindow> DebugWindow::AllDebugWindows;

DebugWindow::DebugWindow(const char* windowName)
    : mWindowName(windowName)
    , mWindowShown()
    , mDebugWindowsListNode(this)
{
    // check name
#ifdef _DEBUG
    for (DebugWindow* currWindow: AllDebugWindows)
    {
        debug_assert(strcmp(currWindow->mWindowName, mWindowName) != 0);
    }
#endif
    AllDebugWindows.insert(&mDebugWindowsListNode);
}

DebugWindow::~DebugWindow()
{
    AllDebugWindows.remove(&mDebugWindowsListNode);
}

void DebugWindow::DoUI(Timespan deltaTime)
{
}
