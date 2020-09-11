#include "stdafx.h"
#include "HumanPlayerView.h"

void HumanPlayerView::UpdateFrame()
{
    if (mCameraController)
    {
        mCameraController->UpdateFrame();
    }

    mOnScreenArea = mCamera.ComputeViewBounds2();
    mHUD.UpdateFrame();
}

void HumanPlayerView::InputEvent(KeyInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayerView::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayerView::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayerView::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayerView::InputEventLost()
{
    if (mCameraController)
    {
        mCameraController->InputEventLost();
    }
}

void HumanPlayerView::SetCameraController(CameraController* controller)
{
    if (mCameraController == controller)
        return;

    mCameraController = controller;
    if (mCameraController)
    {
        mCameraController->Setup(&mCamera);
    }
}