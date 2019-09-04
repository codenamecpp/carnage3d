#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderSystem.h"
#include "SpriteCache.h"
#include "ConsoleWindow.h"

CarnageGame gCarnageGame;

bool CarnageGame::Initialize()
{
    gGameRules.LoadDefaults();
    mCityScape.LoadFromFile("NYC.CMP");

    if (!gSpriteCache.InitLevelSprites(mCityScape.mStyleData))
    {
        debug_assert(false);
    }
    mPedsManager.Initialize();

    // temporary
    glm::vec3 pos { 2.4f, 5.8f, 1.0f };
    mPlayerPedestrian = mPedsManager.CreateRandomPed(pos);

    SetCameraController(&mFollowCameraController);
    return true;
}

void CarnageGame::Deinit()
{
    mPedsManager.Deinit();
    mCityScape.Cleanup();
}

void CarnageGame::UpdateFrame(Timespan deltaTime)
{
    mPedsManager.UpdateFrame(deltaTime);
    if (mCameraController)
    {
        mCameraController->UpdateFrame(deltaTime);
    }
}

void CarnageGame::InputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.mConsumed)
        return;

    if (inputEvent.mKeycode == KEYCODE_TILDE && inputEvent.mPressed) // show debug console
    {
        gDebugConsoleWindow.mWindowShown = true;
        return;
    }
    if (inputEvent.mKeycode == KEYCODE_F3 && inputEvent.mPressed)
    {
        gRenderSystem.ReloadRenderPrograms();
        return;
    }

    if (inputEvent.mKeycode == KEYCODE_LEFT)
    {
        if (mPlayerPedestrian)
        {
            mPlayerPedestrian->mControl.SetTurnLeft(inputEvent.mPressed);
        }
    }

    if (inputEvent.mKeycode == KEYCODE_RIGHT)
    {
        if (mPlayerPedestrian)
        {
            mPlayerPedestrian->mControl.SetTurnRight(inputEvent.mPressed);
        }
    }

    if (inputEvent.mKeycode == KEYCODE_UP)
    {
        if (mPlayerPedestrian)
        {
            mPlayerPedestrian->mControl.SetWalkForward(inputEvent.mPressed);
            mPlayerPedestrian->mControl.SetRunning(inputEvent.mPressed);
        }
    }

    if (inputEvent.mKeycode == KEYCODE_DOWN)
    {
        if (mPlayerPedestrian)
        {
            mPlayerPedestrian->mControl.SetWalkBackward(inputEvent.mPressed);
        }
    }

    if (inputEvent.mKeycode == KEYCODE_ESCAPE && inputEvent.mPressed)
    {
        gSystem.QuitRequest();
    }

    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
}

void CarnageGame::SetCameraController(CameraController* controller)
{
    if (mCameraController == controller)
        return;

    mCameraController = controller;
    if (mCameraController)
    {
        mCameraController->SetupInitial();
    }
}
