#include "stdafx.h"
#include "FollowCameraController.h"
#include "PhysicsComponents.h"
#include "CarnageGame.h"
#include "Pedestrian.h"

FollowCameraController::FollowCameraController()
    : mStartupCameraHeight(8.0f)
    , mFollowPedCameraHeight(5.0f)
    , mFollowPedZoomCameraSpeed(5.0f)
    , mScrollHeightOffset()
{
}

void FollowCameraController::Setup(GameCamera* gameCamera)
{
    debug_assert(gameCamera);
    mCamera = gameCamera;

    // compute aspect ratio
    float screenAspect = (mCamera->mViewportRect.h > 0) ? ((mCamera->mViewportRect.w * 1.0f) / (mCamera->mViewportRect.h * 1.0f)) : 1.0f;

    // set camera defaults
    mCamera->SetIdentity();
    mCamera->SetPerspectiveProjection(screenAspect, 55.0f, 0.1f, 1000.0f);
    
    if (mFollowPedestrian)
    {
        glm::vec3 position = mFollowPedestrian->mPhysicsComponent->GetPosition();
        mCamera->SetPosition({position.x, position.y + mStartupCameraHeight, position.z}); 
    }
    else
    {
        mCamera->SetPosition({0.0f, mStartupCameraHeight, 0.0f});
    }
    mCamera->SetTopDownOrientation();
}

void FollowCameraController::UpdateFrame(Timespan deltaTime)
{
    // correct zoom
    if (mFollowPedestrian)
    {
        glm::vec3 position = mFollowPedestrian->mPhysicsComponent->GetPosition();

        float targetHeight = (position.y + mFollowPedCameraHeight + mScrollHeightOffset);
        float delta = targetHeight - mCamera->mPosition.y;
        if (fabs(delta) > 0.005f)
        {
            position.y = mCamera->mPosition.y + (targetHeight - mCamera->mPosition.y) * mFollowPedZoomCameraSpeed * deltaTime.ToSeconds();
        }
        else
        {
            position.y = targetHeight;
        }
        mCamera->SetPosition(position); 
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

void FollowCameraController::SetFollowTarget(Pedestrian* pedestrian)
{
    mFollowPedestrian = pedestrian;
}
