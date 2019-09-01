#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderSystem.h"
#include "SpriteCache.h"
#include "ConsoleWindow.h"

CarnageGame gCarnageGame;

bool CarnageGame::Initialize()
{
    mTopDownCameraController.SetupInitial();
    mCityScape.LoadFromFile("NYC.CMP");

    if (!gSpriteCache.InitLevelSprites(mCityScape.mStyleData))
    {
        debug_assert(false);
    }
    mPedsManager.Initialize();

    // temporary
    glm::vec3 pos { 2.0f, 2.0f, 2.0f };
    Pedestrian* randomPed = mPedsManager.CreateRandomPed(pos);
    return true;
}

void CarnageGame::Deinit()
{
    mPedsManager.Deinit();
    mCityScape.Cleanup();
}

void CarnageGame::UpdateFrame(Timespan deltaTime)
{
    mTopDownCameraController.UpdateFrame(deltaTime);
    mPedsManager.UpdateFrame(deltaTime);
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

    mTopDownCameraController.InputEvent(inputEvent);
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