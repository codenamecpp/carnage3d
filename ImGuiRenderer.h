#pragma once

#include "imgui.h"

class ImGuiRenderer final: public cxx::noncopyable
{
public:
    // setup internal resources
    bool Initialize();

    // free allocated resources
    void Deinit();

    // render 
    void RenderFrame();

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

public:

private:
    GpuBuffer* mVertexBuffer = nullptr;
    GpuBuffer* mIndexBuffer = nullptr;
};

extern ImGuiRenderer gImGuiRenderer;