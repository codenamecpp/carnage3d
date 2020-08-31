#pragma once

#include "GameCamera.h"
#include "SpriteBatch.h"

// manages all graphical user interface operation
class GuiManager final: public InputEventsHandler
{
public:
    // setup/free internal resources
    bool Initialize();
    void Deinit();

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
    SpriteBatch mSpriteBatch;
    GameCamera2D mCamera2D;
};

extern GuiManager gGuiManager;