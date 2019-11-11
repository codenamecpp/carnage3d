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
static const char* GTA1MapFileExtension = ".CMP";

//////////////////////////////////////////////////////////////////////////

CarnageGame gCarnageGame;

//////////////////////////////////////////////////////////////////////////

bool CarnageGame::Initialize()
{
    gGameCheatsWindow.mWindowShown = true; // show by default

    SetInputActionsFromConfig();

    gGameParams.LoadDefaults();

    // scan all gta1 maps
    std::vector<std::string> gtaMapNames;
    for (const std::string& currSearchPlace: gFiles.mSearchPlaces)
    {
        cxx::enum_files(currSearchPlace, [&gtaMapNames](const std::string& curr)
        {
            if (cxx::get_file_extension(curr) == GTA1MapFileExtension)
            {
                gtaMapNames.push_back(curr);  
            }
        });
    }

    if (gtaMapNames.size())
    {
        gConsole.LogMessage(eLogMessage_Info, "Found GTA1 maps:");
        for (const std::string& currMapname: gtaMapNames)
        {
            gConsole.LogMessage(eLogMessage_Info, " - %s", currMapname.c_str());
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "No GTA1 maps found within search places");
    }

    if (gSystem.mStartupParams.mDebugMapName.empty())
    {
        // try load first found map
        if (gtaMapNames.size())
        {
            gSystem.mStartupParams.mDebugMapName = gtaMapNames[0].c_str();
        }
    }

    if (gSystem.mStartupParams.mDebugMapName.empty())
        return false;

    gGameMap.LoadFromFile(gSystem.mStartupParams.mDebugMapName.c_str());
    gSpriteManager.Cleanup();
    gRenderManager.mMapRenderer.BuildMapMesh();
    if (!gSpriteManager.InitLevelSprites())
    {
        debug_assert(false);
    }
    //gSpriteManager.DumpSpriteDeltas("D:/Temp/gta1_deltas");
    //gSpriteCache.DumpBlocksTexture("D:/Temp/gta1_blocks");
    //gSpriteManager.DumpSpriteTextures("D:/Temp/gta1_sprites");
    //gSpriteManager.DumpCarsTextures("D:/Temp/gta_cars");
    gPhysics.Initialize();

    gGameObjectsManager.Initialize();

    // temporary
    //glm::vec3 pos { 108.0f, 2.0f, 25.0f };
    //glm::vec3 pos { 14.0, 2.0f, 38.0f };
    //glm::vec3 pos { 91.0f, 2.0f, 236.0f };
    //glm::vec3 pos { 121.0f, 2.0f, 200.0f };
    //glm::vec3 pos { 174.0f, 2.0f, 230.0f };

    const int numPlayers = glm::clamp(gSystem.mStartupParams.mPlayersCount, 1, GAME_MAX_PLAYERS);
    gConsole.LogMessage(eLogMessage_Info, "Num players: %d", numPlayers);

    glm::vec3 pos[GAME_MAX_PLAYERS];

    // choose spawn point
    // it is temporary!
    int currFindPosIter = 0;
    for (int yBlock = 10; yBlock < 20 && currFindPosIter < numPlayers; ++yBlock)
    {
        for (int xBlock = 10; xBlock < 20 && currFindPosIter < numPlayers; ++xBlock)
        {
            pos[currFindPosIter] = glm::ivec3(xBlock, MAP_LAYERS_COUNT - 1, yBlock);

            float currHeight = gGameMap.GetHeightAtPosition(pos[currFindPosIter]);
            int zBlock = static_cast<int>(currHeight);
            if (zBlock > MAP_LAYERS_COUNT - 1)
                continue;

            BlockStyle* currBlock = gGameMap.GetBlock(xBlock, yBlock, zBlock);
            if (currBlock->mGroundType == eGroundType_Field ||
                currBlock->mGroundType == eGroundType_Pawement ||
                currBlock->mGroundType == eGroundType_Road)
            {
                pos[currFindPosIter].x += MAP_BLOCK_LENGTH * 0.5f;
                pos[currFindPosIter].z += MAP_BLOCK_LENGTH * 0.5f;
                pos[currFindPosIter].y = currHeight;
                ++currFindPosIter;
            }
        }
    }

    for (int icurr = 0; icurr < numPlayers; ++icurr)
    {
        float randomAngle = 360.0f * gCarnageGame.mGameRand.generate_float();

        Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pos[icurr], cxx::angle_t::from_degrees(randomAngle));
        SetupHumanCharacter(icurr, pedestrian);
    }

    SetupScreenLayout(numPlayers);
    mGameTime = 0;
    return true;
}

void CarnageGame::Deinit()
{
    // temporary
    for (int icurr = 0; icurr < GAME_MAX_PLAYERS; ++icurr)
    {
        if (mHumanSlot[icurr].mCharPedestrian &&
            mHumanSlot[icurr].mCharPedestrian->IsCarPassenger())
        {
            PedestrianStateEvent ev = PedestrianStateEvent::Get_PullOutFromCar(nullptr);
            mHumanSlot[icurr].mCharPedestrian->ProcessEvent(ev);
        }
    }

    gGameObjectsManager.Deinit();
    gPhysics.Deinit();
    gGameMap.Cleanup();
}

void CarnageGame::UpdateFrame(Timespan deltaTime)
{
    // advance game time
    mGameTime += deltaTime;

    gSpriteManager.UpdateBlocksAnimations(deltaTime);
    gPhysics.UpdateFrame(deltaTime);
    gGameObjectsManager.UpdateFrame(deltaTime);

    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanSlot[ihuman].mCharPedestrian == nullptr)
            continue;

        mHumanSlot[ihuman].mCharController.UpdateFrame(mHumanSlot[ihuman].mCharPedestrian, deltaTime);
        mHumanSlot[ihuman].mCharView.UpdateFrame(deltaTime);
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

    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanSlot[ihuman].mCharPedestrian == nullptr)
            continue;

        mHumanSlot[ihuman].mCharController.InputEvent(inputEvent);
        mHumanSlot[ihuman].mCharView.InputEvent(inputEvent);

        if (inputEvent.mConsumed)
            break;
    }
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanSlot[ihuman].mCharPedestrian == nullptr)
            continue;

        mHumanSlot[ihuman].mCharView.InputEvent(inputEvent);

        if (inputEvent.mConsumed)
            break;
    }
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanSlot[ihuman].mCharPedestrian == nullptr)
            continue;

        mHumanSlot[ihuman].mCharView.InputEvent(inputEvent);

        if (inputEvent.mConsumed)
            break;
    }
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanSlot[ihuman].mCharPedestrian == nullptr)
            continue;

        mHumanSlot[ihuman].mCharView.InputEvent(inputEvent);

        if (inputEvent.mConsumed)
            break;
    }
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
}

void CarnageGame::InputEvent(GamepadInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanSlot[ihuman].mCharPedestrian == nullptr)
            continue;

        mHumanSlot[ihuman].mCharController.InputEvent(inputEvent);

        if (inputEvent.mConsumed)
            break;
    }
}

bool CarnageGame::SetInputActionsFromConfig()
{
    // force default mapping for first player
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        HumanCharacterSlot& currentChar = mHumanSlot[ihuman];
        currentChar.mCharController.mInputs.SetNull();
        if (ihuman == 0) 
        {
            currentChar.mCharController.mInputs.SetDefaults();
        }  
    }

    // open config document
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
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        HumanCharacterController& currentChar = mHumanSlot[ihuman].mCharController;

        tempString.printf("player%d", ihuman + 1);

        cxx::config_node configNode = configDocument.get_root_node().get_child(tempString.c_str());
        currentChar.mInputs.SetFromConfig(configNode);
    }

    return true;
}

void CarnageGame::SetupHumanCharacter(int humanIndex, Pedestrian* pedestrian)
{
    // todo: what a mess
    debug_assert(humanIndex < GAME_MAX_PLAYERS);
    debug_assert(pedestrian);
    debug_assert(mHumanSlot[humanIndex].mCharPedestrian == nullptr);
    if (mHumanSlot[humanIndex].mCharPedestrian)
        return;

    if (humanIndex > 0)
    {
        pedestrian->mRemapIndex = humanIndex - 1;
    }
    mHumanSlot[humanIndex].mCharController.mSpawnPosition = pedestrian->mPhysicsComponent->GetPosition();
    mHumanSlot[humanIndex].mCharPedestrian = pedestrian;
    mHumanSlot[humanIndex].mCharController.SetCharacter(pedestrian);
    mHumanSlot[humanIndex].mCharView.mFollowCameraController.SetFollowTarget(pedestrian);
}

void CarnageGame::SetupScreenLayout(int playersCount)
{   
    // todo: what a mess
    const int MaxCols = 2;

    Rect2D fullViewport = gGraphicsDevice.mViewportRect;

    int numRows = (playersCount + MaxCols - 1) / MaxCols;
    debug_assert(numRows > 0);

    int frameSizePerH = fullViewport.h / numRows;

    for (int icurr = 0; icurr < playersCount; ++icurr)
    {
        int currRow = icurr / MaxCols;
        int currCol = icurr % MaxCols;

        int colsOnCurrentRow = glm::clamp(playersCount - (currRow * MaxCols), 1, MaxCols);
        debug_assert(colsOnCurrentRow);
        int frameSizePerW = fullViewport.w / colsOnCurrentRow;
        
        mHumanSlot[icurr].mCharView.mRenderCamera.mViewportRect.h = frameSizePerH;
        mHumanSlot[icurr].mCharView.mRenderCamera.mViewportRect.x = currCol * (frameSizePerW + 1);
        mHumanSlot[icurr].mCharView.mRenderCamera.mViewportRect.y = (numRows - currRow - 1) * (frameSizePerH + 1);
        mHumanSlot[icurr].mCharView.mRenderCamera.mViewportRect.w = frameSizePerW;
 
        mHumanSlot[icurr].mCharView.SetCameraController(&mHumanSlot[icurr].mCharView.mFollowCameraController);
        gRenderManager.AttachRenderView(&mHumanSlot[icurr].mCharView);
    }
}
