#pragma once

// forwards
struct ImGuiIO;

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
    // @param imguiContext: Internal imgui context
    virtual void DoUI(ImGuiIO& imguiContext) = 0;

    inline void ToggleWindowShown()
    {
        mWindowShown = !mWindowShown;
    }

public:
    const char* mWindowName;
    bool mWindowShown;
};