#include "stdafx.h"
#include "FollowCameraController.h"
#include "PhysicsObject.h"

FollowCameraController::FollowCameraController()
    : mStartupCameraHeight(8.0f)
    , mFollowPedCameraHeight(5.0f)
    , mFollowPedZoomCameraSpeed(5.0f)
    , mScrollHeightOffset()
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
        gCamera.SetPosition({position.x, position.y + mStartupCameraHeight, position.z}); 
    }
    else
    {
        gCamera.SetPosition({0.0f, mStartupCameraHeight, 0.0f});
    }
    gCamera.SetTopDownOrientation();
}

void FollowCameraController::UpdateFrame(Timespan deltaTime)
{
    // correct zoom
    if (Pedestrian* player = gCarnageGame.mPlayerPedestrian)
    {
        glm::vec3 position = player->mPhysicalBody->GetPosition();

        float targetHeight = (position.y + mFollowPedCameraHeight + mScrollHeightOffset);
        float delta = targetHeight - gCamera.mPosition.y;
        if (fabs(delta) > 0.005f)
        {
            position.y = gCamera.mPosition.y + (targetHeight - gCamera.mPosition.y) * mFollowPedZoomCameraSpeed * deltaTime.ToSeconds();
        }
        else
        {
            position.y = targetHeight;
        }
        gCamera.SetPosition(position); 
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
    mScrollHeightOffset -= inputEvent.mScrollY;
}