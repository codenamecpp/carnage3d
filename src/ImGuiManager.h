#pragma once

#include "imgui.h"
#include "TrimeshBuffer.h"

class GuiRenderContext;
class DebugWindow;

class ImGuiManager final: public cxx::noncopyable
{
public:
    // setup internal resources
    bool Initialize();

    // free allocated resources
    void Deinit();

    // render 
    void RenderFrame(GuiRenderContext& renderContext);

    // process logic
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(Timespan deltaTime);

    // process input events
    // @param inputEvent: Input event data
    void HandleEvent(MouseMovedInputEvent& inputEvent);
    void HandleEvent(MouseScrollInputEvent& inputEvent);
    void HandleEvent(MouseButtonInputEvent& inputEvent);
    void HandleEvent(KeyInputEvent& inputEvent);
    void HandleEvent(KeyCharEvent& inputEvent);

    bool IsInitialized() const;

private:
    // internals
    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    TrimeshBuffer mTrimeshBuffer;
};

extern ImGuiManager gImGuiManager;