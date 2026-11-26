#include "stdafx.h"
#include "FollowCameraController.h"
#include "PhysicsBody.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"

//////////////////////////////////////////////////////////////////////////

namespace
{
    const float StartupCameraHeight = 32.0f;

    const float FollowPedCameraHeight = 20.0f;
    const float FollowCarMinCameraHeight = 20.0f;
    const float FollowCarMaxCameraHeight = 32.0f;
    const float CameraZoomCarSpeedTreshold = 10.0f;
    const float CameraZoomCarSpeed = 30.0f;

    const float CameraLookAheadDistance = 12.0f;

    const float CameraCatchHeightStiffness = 2.0f;
    const float CameraCatchPedPositionStiffness = 10.0f;
    const float CameraCatchCarPositionStiffness = 5.0f;
}

//////////////////////////////////////////////////////////////////////////

FollowCameraController::FollowCameraController()
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
    mCamera->SetPerspectiveProjection(screenAspect, 55.0f, 0.1f, 300.0f);
    
    if (mFollowPedestrian)
    {
        glm::vec3 position = mFollowPedestrian->mTransformSmooth.mPosition;
        mCamera->SetPosition({position.x, position.y + StartupCameraHeight, position.z}); 
    }
    else
    {
        mCamera->SetPosition({0.0f, StartupCameraHeight, 0.0f});
    }
    mCamera->SetTopDownOrientation();
}

void FollowCameraController::UpdateFrame()
{
    if (mFollowPedestrian == nullptr)
        return;

    glm::vec3 actorPosition = mFollowPedestrian->mTransformSmooth.mPosition;
    glm::vec2 desiredOffset {0.0f};
    float desiredHeight = FollowPedCameraHeight;
    float catchPositionStiffness = CameraCatchPedPositionStiffness;

    const bool isInCar = mFollowPedestrian->IsCarPassenger();
    if (isInCar)
    {
        Vehicle* currentCar = mFollowPedestrian->mCurrentCar;
        debug_assert(currentCar);

        actorPosition = currentCar->mTransformSmooth.mPosition;
        desiredHeight = FollowCarMinCameraHeight;

        const float carSignedSpeed = currentCar->GetCurrentSpeed();
        const float carSpeed = fabs(carSignedSpeed);
        if (carSpeed > CameraZoomCarSpeedTreshold)
        {
            // handle zoom
            const float zoomFactor = std::min((carSpeed - CameraZoomCarSpeedTreshold) / (CameraZoomCarSpeed - CameraZoomCarSpeedTreshold), 1.0f);
            desiredHeight = glm::lerp(FollowCarMinCameraHeight, FollowCarMaxCameraHeight, zoomFactor);
        }

        catchPositionStiffness = CameraCatchCarPositionStiffness;

        if (carSpeed > 0.0f)
        {
            const float speedSign = glm::sign(carSignedSpeed);
            desiredOffset = currentCar->mTransformSmooth.GetDirectionVector() * speedSign * CameraLookAheadDistance *
                glm::clamp((desiredHeight - FollowCarMinCameraHeight) / (FollowCarMaxCameraHeight - FollowCarMinCameraHeight), 0.0f, 1.0f);
        }
    }

    const glm::vec3 desiredCameraPosition
    {
        actorPosition.x + desiredOffset.x,
        actorPosition.y + desiredHeight,
        actorPosition.z + desiredOffset.y
    };

    const float deltaTime = gTimeManager.mGameFrameDelta;

    const glm::vec3 cameraPosition
    {
        cxx::damp(mCamera->mPosition.x, desiredCameraPosition.x, catchPositionStiffness, deltaTime),
        cxx::damp(mCamera->mPosition.y, desiredCameraPosition.y, CameraCatchHeightStiffness, deltaTime),
        cxx::damp(mCamera->mPosition.z, desiredCameraPosition.z, catchPositionStiffness, deltaTime)
    };

    mCamera->SetPosition(cameraPosition); 
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

void FollowCameraController::SetFollowTarget(Pedestrian* pedestrian)
{
    mFollowPedestrian = pedestrian;
}

void FollowCameraController::InputEventLost()
{
}
