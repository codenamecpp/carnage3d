#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderSystem.h"

CarnageGame gCarnageGame;

bool CarnageGame::Initialize()
{
    mTopDownCameraController.SetupInitial();
    mCityScape.LoadFromFile("NYC.CMP");

    return true;
}

void CarnageGame::Deinit()
{
    mCityScape.Cleanup();
}

void CarnageGame::UpdateFrame(Timespan deltaTime)
{
    mTopDownCameraController.UpdateFrame(deltaTime);
}

void CarnageGame::InputEvent(KeyInputEvent& inputEvent)
{
    if (!inputEvent.mConsumed)
    {
        if (inputEvent.mKeycode == KEYCODE_TILDE) // show debug console
        {
            gGuiSystem.SetShowDebugConsole(true);
        }
        else
        {
            mTopDownCameraController.InputEvent(inputEvent);
        }
    }
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mTopDownCameraController.InputEvent(inputEvent);
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mTopDownCameraController.InputEvent(inputEvent);
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mTopDownCameraController.InputEvent(inputEvent);
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
}