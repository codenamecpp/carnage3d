#include "stdafx.h"
#include "FollowCameraController.h"
#include "PhysicsObject.h"

FollowCameraController::FollowCameraController()
    : mStartupCameraHeight(12.0f)
    , mFollowPedCameraHeight(5.0f)
    , mFollowPedZoomCameraSpeed(1.0f)
{
}

void FollowCameraController::SetupInitial()
{
    // set camera defaults
    gCamera.SetIdentity();
    gCamera.SetPerspectiveProjection(gSystem.mConfig.mScreenAspectRatio, 55.0f, 0.1f, 1000.0f);
    
    if (Pedestrian* player = gCarnageGame.mPlayerPedestrian)
    {
        glm::vec3 position = player->mPhysicalBody->GetPosition();
        gCamera.SetPosition({position.x, mStartupCameraHeight, position.z}); 
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
        glm::vec3 position = player->mPhysicalBody->GetPosition();
        float targetHeight = (position.y + mFollowPedCameraHeight);
        if (fabs(targetHeight - gCamera.mPosition.y) > 0.1f)
        {
            mMoveDirection.y = (targetHeight - gCamera.mPosition.y) * deltaTime.ToSeconds();
        }
        gCamera.SetPosition({position.x, mFollowPedCameraHeight, position.z}); 
    }
    
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
    mFollowPedCameraHeight -= inputEvent.mScrollY;
}