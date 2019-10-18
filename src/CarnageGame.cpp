#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "ConsoleWindow.h"
#include "GameCheatsWindow.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "MemoryManager.h"

CarnageGame gCarnageGame;

bool CarnageGame::Initialize()
{
    gGameCheatsWindow.mWindowShown = true; // show by default

    gGameRules.LoadDefaults();
    if (gSystem.mStartupParams.mDebugMapName.empty())
    {
        gSystem.mStartupParams.mDebugMapName.set_content("NYC.CMP");
    }

    gGameMap.LoadFromFile(gSystem.mStartupParams.mDebugMapName.c_str());
    gSpriteManager.Cleanup();
    gRenderManager.mMapRenderer.InvalidateMapMesh();
    if (!gSpriteManager.InitLevelSprites())
    {
        debug_assert(false);
    }
    //gSpriteManager.DumpSpriteDeltas("D:/Temp/gta1_deltas");
    //gSpriteCache.DumpBlocksTexture("D:/Temp/gta1_blocks");
    //gSpriteManager.DumpSpriteTextures("D:/Temp/gta1_sprites");
    //gSpriteManager.DumpCarsTextures("D:/Temp/gta_cars");
    gPhysics.Initialize();

    mObjectsManager.Initialize();

    // temporary
    //glm::vec3 pos { 108.0f, 2.0f, 25.0f };
    //glm::vec3 pos { 14.0, 2.0f, 38.0f };
    //glm::vec3 pos { 91.0f, 2.0f, 236.0f };
    //glm::vec3 pos { 121.0f, 2.0f, 200.0f };
    //glm::vec3 pos { 174.0f, 2.0f, 230.0f };
    glm::vec3 pos { 5.0f, 5.0f, 7.0f };
    pos.y = gGameMap.GetHeightAtPosition(pos);
    mPlayerPedestrian = mObjectsManager.CreatePedestrian(pos);
    mHumanController.SetCharacter(mPlayerPedestrian);

    for (int icartype = 0; icartype < gGameMap.mStyleData.mCars.size(); ++icartype)
    {
        Vehicle* dummyCar = mObjectsManager.CreateCar(pos + glm::vec3 {4.0f + icartype * 1.3f, 0.0f, 2.0f}, &gGameMap.mStyleData.mCars[icartype]);
        dummyCar->mPhysicsComponent->SetRotationAngle(cxx::angle_t::from_degrees(icartype * 60.0f));
    }

    SetCameraController(&mFollowCameraController);

    mGameTime = 0;
    return true;
}

void CarnageGame::Deinit()
{
    mObjectsManager.Deinit();
    gPhysics.Deinit();
    gGameMap.Cleanup();
}

void CarnageGame::UpdateFrame(Timespan deltaTime)
{
    // advance game time
    mGameTime += deltaTime;

    gSpriteManager.UpdateBlocksAnimations(deltaTime);
    gPhysics.UpdateFrame(deltaTime);
    mObjectsManager.UpdateFrame(deltaTime);
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
        gDebugConsoleWindow.mWindowShown = !gDebugConsoleWindow.mWindowShown;
        return;
    }
    if (inputEvent.mKeycode == KEYCODE_F3 && inputEvent.mPressed)
    {
        gRenderManager.ReloadRenderPrograms();
        return;
    }

    mHumanController.InputEvent(inputEvent);

    if (inputEvent.mKeycode == KEYCODE_ESCAPE && inputEvent.mPressed)
    {
        gSystem.QuitRequest();
    }

    if (inputEvent.mKeycode == KEYCODE_C && inputEvent.mPressed)
    {
        gGameCheatsWindow.mWindowShown = !gGameCheatsWindow.mWindowShown;
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
