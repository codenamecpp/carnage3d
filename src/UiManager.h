#pragma once

#include "GameCamera.h"
#include "SpriteBatch.h"

// manages all graphical user interface operation
class UiManager final: public cxx::noncopyable
{
public:
    // setup/free internal resources
    bool Initialize();
    void Deinit();

    // render
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

private:
    SpriteBatch mSpriteBatch;
    GameCamera2D mCamera2D;
};

extern UiManager gUiManager;