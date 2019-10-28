#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "ConsoleWindow.h"
#include "GameCheatsWindow.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "MemoryManager.h"

static const char* InputsConfigPath = "config/inputs.json";

//////////////////////////////////////////////////////////////////////////

CarnageGame gCarnageGame;

//////////////////////////////////////////////////////////////////////////

bool CarnageGame::Initialize()
{
    gGameCheatsWindow.mWindowShown = true; // show by default

    SetInputActionsFromConfig();

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
    glm::vec3 pos;

    // choose spawn point
    // it is temporary!
    for (int yBlock = 10; yBlock < 20; ++yBlock)
    {
        for (int xBlock = 10; xBlock < 20; ++xBlock)
        {
            pos = glm::ivec3(xBlock, MAP_LAYERS_COUNT - 1, yBlock);

            float currHeight = gGameMap.GetHeightAtPosition(pos);
            int zBlock = static_cast<int>(currHeight);
            if (zBlock > MAP_LAYERS_COUNT - 1)
                continue;

            BlockStyle* currBlock = gGameMap.GetBlock(xBlock, yBlock, zBlock);
            if (currBlock->mGroundType == eGroundType_Field ||
                currBlock->mGroundType == eGroundType_Pawement ||
                currBlock->mGroundType == eGroundType_Road)
            {
                pos.x += MAP_BLOCK_LENGTH * 0.5f;
                pos.z += MAP_BLOCK_LENGTH * 0.5f;
                pos.y = currHeight;
                break;
            }
        }
    }

    mPlayerPedestrian = mObjectsManager.CreatePedestrian(pos);
    mHumanCharacters[0].SetCharacter(mPlayerPedestrian);

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

    if (inputEvent.mKeycode == eKeycode_TILDE && inputEvent.mPressed) // show debug console
    {
        gDebugConsoleWindow.mWindowShown = !gDebugConsoleWindow.mWindowShown;
        return;
    }
    if (inputEvent.mKeycode == eKeycode_F3 && inputEvent.mPressed)
    {
        gRenderManager.ReloadRenderPrograms();
        return;
    }

    if (inputEvent.mKeycode == eKeycode_ESCAPE && inputEvent.mPressed)
    {
        gSystem.QuitRequest();
        return;
    }

    if (inputEvent.mKeycode == eKeycode_C && inputEvent.mPressed)
    {
        gGameCheatsWindow.mWindowShown = !gGameCheatsWindow.mWindowShown;
        return;
    }

    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }

    for (HumanCharacterController& currentController: mHumanCharacters)
    {
        if (inputEvent.mConsumed)
            break;

        currentController.InputEvent(inputEvent);
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

void CarnageGame::InputEvent(GamepadInputEvent& inputEvent)
{
    for (int icurr = 0; icurr < GAME_MAX_PLAYERS; ++icurr)
    {
        if (inputEvent.mConsumed)
            break;

        mHumanCharacters[icurr].InputEvent(inputEvent);
    }
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

bool CarnageGame::SetInputActionsFromConfig()
{
    std::string jsonContent;
    if (!gFiles.ReadTextFile(InputsConfigPath, jsonContent))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load input config from '%s'", InputsConfigPath);
        return false;
    }

    cxx::config_document configDocument;
    if (!configDocument.parse_document(jsonContent.c_str()))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse input config document");
        return false;
    }

    cxx::string_buffer_32 tempString;
    for (int iplayer = 0; iplayer < GAME_MAX_PLAYERS; ++iplayer)
    {
        HumanCharacterController& currentChar = mHumanCharacters[iplayer];
        currentChar.mInputActionsMapping.SetNull();

        if (iplayer == 0) // force default mapping for first player
        {
            currentChar.mInputActionsMapping.SetDefaults();
        }

        tempString.printf("player%d", iplayer + 1);

        cxx::config_node configNode = configDocument.get_root_node().get_child(tempString.c_str());
        currentChar.mInputActionsMapping.SetFromConfig(configNode);
    }
    return true;
}