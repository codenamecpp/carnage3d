#pragma once

#include "GameDefs.h"
#include "FollowCameraController.h"
#include "FreeLookCameraController.h"
#include "RenderView.h"

class HumanCharacterView: public RenderView
{
public:
    HumanCharacterView();

    void UpdateFrame(Timespan deltaTime);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);

    // public for debug purposes
    void SetCameraController(CameraController* controller);

public:
    FollowCameraController mFollowCameraController;
    FreeLookCameraController mFreeLookCameraController;
    CameraController* mCameraController = nullptr;
};