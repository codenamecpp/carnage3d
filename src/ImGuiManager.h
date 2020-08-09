#pragma once

#include "imgui.h"
#include "TrimeshBuffer.h"

class ImGuiManager final: public cxx::noncopyable
{
public:
    // setup/free internal resources
    bool Initialize();
    void Deinit();
    bool IsInitialized() const;

    // render 
    void RenderFrame();

    // process logic
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(Timespan deltaTime);

    // process input events
    // @param inputEvent: Input event data
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);

private:
    // internals
    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    TrimeshBuffer mTrimeshBuffer;
};

extern ImGuiManager gImGuiManager;