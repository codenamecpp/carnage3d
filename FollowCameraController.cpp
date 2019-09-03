#include "stdafx.h"
#include "FollowCameraController.h"

FollowCameraController::FollowCameraController()
    : mStartupCameraHeight(12.0f)
    , mFollowPedCameraHeight(4.0f)
    , mFollowPedZoomCameraSpeed(1.0f)
{
}

void FollowCameraController::SetupInitial()
{
    // set camera defaults
    gCamera.SetIdentity();
    gCamera.SetPerspectiveProjection(gSystem.mConfig.mScreenAspectRatio, 55.0f, 0.1f, 100.0f);
    
    if (Pedestrian* player = gCarnageGame.mPlayerPedestrian)
    {
        gCamera.SetPosition({player->mPosition.x, mStartupCameraHeight, player->mPosition.y}); 
    }
    else
    {
        gCamera.SetPosition({0.0f, mStartupCameraHeight, 0.0f});
    }
    gCamera.SetTopDownOrientation();
}

void FollowCameraController::UpdateFrame(Timespan deltaTime)
{
    mMoveDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    // correct zoom
    if (Pedestrian* player = gCarnageGame.mPlayerPedestrian)
    {
        float targetHeight = (player->mPosition.z + mFollowPedCameraHeight);
        if (fabs(targetHeight) > 1.0f)
        {
            mMoveDirection.y = (targetHeight - gCamera.mPosition.y) * deltaTime.ToSeconds();
        }
    }
    gCamera.Translate(mMoveDirection); 
}

void FollowCameraController::InputEvent(KeyInputEvent& inputEvent)
{
}

void FollowCameraController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void FollowCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void FollowCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
}