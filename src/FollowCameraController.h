#pragma once

#include "CameraController.h"

class FollowCameraController final: public CameraController
{
public:
    FollowCameraController();

    void SetFollowTarget(Pedestrian* pedestrian);

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
    // parameters
    float mStartupCameraHeight; // meters
    float mFollowPedCameraHeight; // meters
    float mScrollHeightOffset; // meters
    float mFollowPedCameraCatchSpeed; // meters per second

    Pedestrian* mFollowPedestrian = nullptr;
};