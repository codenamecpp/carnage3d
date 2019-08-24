#pragma once

#include "GuiRenderContext.h"

// manages all graphical user interface operation
class GuiSystem final: public cxx::noncopyable
{
public:
    GuiRenderContext mRenderContext;

public:
    // setup internal resources
    bool Initialize();

    // free allocated resources
    void Deinit();

    // render all
    void RenderFrame();

    // process gui logic
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(Timespan deltaTime);

    // process input events
    // @param inputEvent: Input event data
    void HandleEvent(MouseMovedInputEvent& inputEvent);
    void HandleEvent(MouseScrollInputEvent& inputEvent);
    void HandleEvent(MouseButtonInputEvent& inputEvent);
    void HandleEvent(KeyInputEvent& inputEvent);
    void HandleEvent(KeyCharEvent& inputEvent);

};

extern GuiSystem gGuiSystem;