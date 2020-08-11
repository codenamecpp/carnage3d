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

    void RenderFrame();
    void UpdateFrame();

    // process input events
    // @param inputEvent: Input event data
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);

private:
    SpriteBatch mSpriteBatch;
    GameCamera2D mCamera2D;
};

extern UiManager gUiManager;