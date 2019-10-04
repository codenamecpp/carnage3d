#include "stdafx.h"
#include "DebugWindow.h"
#include "ImGuiManager.h"

DebugWindow::DebugWindow(const char* windowName)
    : mWindowName(windowName)
    , mWindowShown()
    , mDebugWindowsListNode(this)
{
    // check name
#ifdef _DEBUG
    for (DebugWindow* currWindow: GetDebugWindowsList())
    {
        debug_assert(strcmp(currWindow->mWindowName, mWindowName) != 0);
    }
#endif
    GetDebugWindowsList().insert(&mDebugWindowsListNode);
}

DebugWindow::~DebugWindow()
{
    GetDebugWindowsList().remove(&mDebugWindowsListNode);
}

void DebugWindow::DoUI(Timespan deltaTime)
{
}
