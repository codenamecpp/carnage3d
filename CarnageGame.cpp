#include "stdafx.h"
#include "CarnageGame.h"

CarnageGame gCarnageGame;

bool CarnageGame::Initialize()
{
    // todo: move camera initialization to game state
    gCamera.SetIdentity();
    //gCamera.SetPerspectiveProjection(gSystem.mConfig.mScreenAspectRatio, 45.0f, 0.01f, 1000.0f);
    gCamera.SetOrthographicProjection(0.0, 1000.0, 1000.0, 0.0f);

    mCityScape.LoadFromFile("NYC.CMP");

    return true;
}

void CarnageGame::Deinit()
{
    mCityScape.Cleanup();
}

void CarnageGame::UpdateFrame(Timespan deltaTime)
{
}

void CarnageGame::InputEvent(KeyInputEvent& inputEvent)
{
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
}