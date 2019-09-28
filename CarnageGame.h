#pragma once

#include "GameMapManager.h"
#include "FollowCameraController.h"
#include "FreeLookCameraController.h"
#include "GameObjectsManager.h"

// top level game application controller
class CarnageGame final: public cxx::noncopyable
{
public:
    GameObjectsManager mObjectsManager;
    FollowCameraController mFollowCameraController;
    FreeLookCameraController mFreeLookCameraController;
    // gamestate
    Pedestrian* mPlayerPedestrian = nullptr;
    CameraController* mCameraController = nullptr;

    Timespan mGameTime;

public:
    // Setup resources and switch to initial game state
    bool Initialize();

    // Cleanup current state and finish game
    void Deinit();

    // Common processing
    void UpdateFrame(Timespan deltaTime);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

    // public for debug purposes
    void SetCameraController(CameraController* controller);

private:

};

extern CarnageGame gCarnageGame;