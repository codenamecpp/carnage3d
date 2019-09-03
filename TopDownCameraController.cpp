#include "stdafx.h"
#include "TopDownCameraController.h"

void TopDownCameraController::SetupInitial()
{
    gCamera.SetIdentity();
    gCamera.SetPerspectiveProjection(gSystem.mConfig.mScreenAspectRatio, 55.0f, 0.1f, 100.0f);
    //gCamera.SetPosition({92.8068237f, 24.3673191f, 122.715485f});
    gCamera.SetPosition({0.0f, 24.0f, 0.0f});
    gCamera.SetTopDownOrientation();
}

void TopDownCameraController::UpdateFrame(Timespan deltaTime)
{
    mMoveDirection = {};

    if (gInputs.GetKeyState(KEYCODE_W) || gInputs.GetKeyState(KEYCODE_S)) 
    {
        float moveSign = gInputs.GetKeyState(KEYCODE_W) ? 1.0f : -1.0f;
        mMoveDirection += gCamera.mUpDirection * moveSign;
    }

    if (gInputs.GetKeyState(KEYCODE_A) || gInputs.GetKeyState(KEYCODE_D)) 
    {
        float moveSign = gInputs.GetKeyState(KEYCODE_A) ? -1.0f : 1.0f;
        mMoveDirection += gCamera.mRightDirection * moveSign;
    }

    if (gInputs.GetKeyState(KEYCODE_PAGE_UP) || gInputs.GetKeyState(KEYCODE_PAGE_DOWN)) 
    {
        float moveSign = gInputs.GetKeyState(KEYCODE_PAGE_UP) ? 1.0f : -1.0f;
        mMoveDirection += gCamera.mFrontDirection * moveSign;
    }

    mMoveDirection *= 18.5f; // speed
    gCamera.Translate(mMoveDirection * deltaTime.ToSeconds());
}

void TopDownCameraController::InputEvent(KeyInputEvent& inputEvent)
{
}

void TopDownCameraController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void TopDownCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void TopDownCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
}