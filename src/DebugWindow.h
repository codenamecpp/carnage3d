#pragma once

// base class for all debug windows
class DebugWindow: public cxx::noncopyable
{
    friend class ImGuiManager;

    static cxx::intrusive_list<DebugWindow>& GetDebugWindowsList()
    {
        static cxx::intrusive_list<DebugWindow> AllDebugWindows;
        return AllDebugWindows;
    }

    // linked list of all debug windows exists in program
    cxx::intrusive_node<DebugWindow> mDebugWindowsListNode;

public: 
    // @param windowName: Unique name, must be statically allocated
    DebugWindow(const char* windowName);
    virtual ~DebugWindow();
    
    // process window state
    // @param deltaTime: Time since last frame
    virtual void DoUI(Timespan deltaTime);

public:
    const char* mWindowName;
    bool mWindowShown;
};