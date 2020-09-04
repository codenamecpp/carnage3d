#include "stdafx.h"
#include "HumanCharacterView.h"

void HumanCharacterView::UpdateFrame()
{
    if (mCameraController)
    {
        mCameraController->UpdateFrame();
    }

    mOnScreenArea = mCamera.ComputeViewBounds2();
    mHUD.UpdateFrame();
}

void HumanCharacterView::InputEvent(KeyInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanCharacterView::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanCharacterView::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanCharacterView::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanCharacterView::InputEventLost()
{
    if (mCameraController)
    {
        mCameraController->InputEventLost();
    }
}

void HumanCharacterView::SetCameraController(CameraController* controller)
{
    if (mCameraController == controller)
        return;

    mCameraController = controller;
    if (mCameraController)
    {
        mCameraController->Setup(&mCamera);
    }
}