#pragma once

#include "imgui.h"
#include "TrimeshBuffer.h"

class ImGuiManager final: public InputEventsHandler
{
public:
    // setup/free internal resources
    bool Initialize();
    void Deinit();
    bool IsInitialized() const;

    void RenderFrame();
    void UpdateFrame();

    // override InputEventsHandler
    void InputEvent(MouseMovedInputEvent& inputEvent) override;
    void InputEvent(MouseScrollInputEvent& inputEvent) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(KeyCharEvent& inputEvent) override;
    void InputEvent(GamepadInputEvent& inputEvent) override;

private:
    // internals
    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    TrimeshBuffer mTrimeshBuffer;
};

extern ImGuiManager gImGuiManager;