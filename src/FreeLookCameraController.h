#pragma once

#include "CameraController.h"

class FreeLookCameraController final: public CameraController
{
public:
    // reset scene camera to defaults
    void Setup(GameCamera* gameCamera) override;

    // process events and advance controller logic for single frame
    // @param deltaTime: Time since last frame
    void UpdateFrame(Timespan deltaTime) override;

    // process input event
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(MouseMovedInputEvent& inputEvent) override;
    void InputEvent(MouseScrollInputEvent& inputEvent) override;

private:
    bool mMoveLeft, mMoveRight;
    bool mMoveForward, mMoveBackward;
    int mLastMouseX, mLastMouseY;
    int mRotateDeltaX, mRotateDeltaY;
    bool mMouseDragCamera;
};
