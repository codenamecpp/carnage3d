#pragma once

#include "CameraController.h"

class FreeLookCameraController final: public CameraController
{
public:
    // reset scene camera to defaults
    void Setup(GameCamera* gameCamera) override;

    // process events and advance controller logic for single frame
    void UpdateFrame() override;

    // process input event
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(MouseMovedInputEvent& inputEvent) override;
    void InputEvent(MouseScrollInputEvent& inputEvent) override;
    void InputEventLost() override;

private:
    void ClearConstrols();

private:
    bool mMoveLeft, mMoveRight;
    bool mMoveForward, mMoveBackward;
    int mLastMouseX, mLastMouseY;
    int mRotateDeltaX, mRotateDeltaY;
    bool mMouseDragCamera;
    float mMoveSpeed = 20.0f; // meters per second
};
