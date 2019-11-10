#include "stdafx.h"
#include "FollowCameraController.h"
#include "PhysicsComponents.h"
#include "CarnageGame.h"
#include "Pedestrian.h"

FollowCameraController::FollowCameraController()
    : mStartupCameraHeight(8.0f)
    , mFollowPedCameraHeight(5.0f)
    , mFollowPedCameraCatchSpeed(5.0f)
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
    if (mFollowPedestrian == nullptr)
        return;

    glm::vec3 position = mFollowPedestrian->mPhysicsComponent->GetPosition();
    position.y = position.y + (mFollowPedCameraHeight + mScrollHeightOffset);

    float catchSpeed = mFollowPedCameraCatchSpeed;
    // todo: temporary implementation
    if (mFollowPedestrian->IsCarPassenger())
    {
        glm::vec2 carVelocity = mFollowPedestrian->mCurrentCar->mPhysicsComponent->GetLinearVelocity();
        float carSpeed = glm::length(carVelocity);
        carVelocity = glm::normalize(carVelocity);
        position.x += (carVelocity.x * carSpeed * 0.35f);
        position.z += (carVelocity.y * carSpeed * 0.35f);
    }

    if (glm::length(mCamera->mPosition - position) > 0.01f)
    {
        position = mCamera->mPosition + (position - mCamera->mPosition) * catchSpeed * deltaTime.ToSeconds();
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
    mScrollHeightOffset = glm::clamp(mScrollHeightOffset - inputEvent.mScrollY, -3.0f, 23.0f);
}

void FollowCameraController::SetFollowTarget(Pedestrian* pedestrian)
{
    mFollowPedestrian = pedestrian;
}
