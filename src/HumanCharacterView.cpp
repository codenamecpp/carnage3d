#include "stdafx.h"
#include "HumanCharacterView.h"

HumanCharacterView::HumanCharacterView()
    : mFollowCameraController()
    , mFreeLookCameraController()
{
}

void HumanCharacterView::UpdateFrame(Timespan deltaTime)
{
    if (mCameraController)
    {
        mCameraController->UpdateFrame(deltaTime);
    }
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

void HumanCharacterView::SetCameraController(CameraController* controller)
{
    if (mCameraController == controller)
        return;

    mCameraController = controller;
    if (mCameraController)
    {
        mCameraController->Setup(&mRenderCamera);
    }
}