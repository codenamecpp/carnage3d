#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "ConsoleWindow.h"
#include "GameCheatsWindow.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "MemoryManager.h"
#include "TimeManager.h"
#include "TrafficManager.h"
#include "AiManager.h"
#include "GameTextsManager.h"
#include "BroadcastEventsManager.h"

static const char* InputsConfigPath = "config/inputs.json";

//////////////////////////////////////////////////////////////////////////

CarnageGame gCarnageGame;

//////////////////////////////////////////////////////////////////////////

bool CarnageGame::Initialize()
{
    ::memset(mHumanPlayers, 0, sizeof(mHumanPlayers));

    gGameCheatsWindow.mWindowShown = true; // show by default

    gGameParams.SetToDefaults();

    if (gSystem.mStartupParams.mDebugMapName.empty())
    {
        // try load first found map
        debug_assert(!gFiles.mGameMapsList.empty());
        gSystem.mStartupParams.mDebugMapName = gFiles.mGameMapsList[0];
    }

    if (gSystem.mStartupParams.mDebugMapName.empty())
        return false;

    // init randomizer
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    mGameRand.set_seed((unsigned int) ms.count());

    // init game texts
    gGameTexts.Initialize();
    gGameTexts.LoadTexts("ENGLISH.FXT");

    // init scenario
    if (!StartScenario(gSystem.mStartupParams.mDebugMapName))
    {
        ShutdownCurrentScenario();

        gConsole.LogMessage(eLogMessage_Warning, "Fail to start game"); 
        return false;
    }
    return true;
}

void CarnageGame::Deinit()
{
    ShutdownCurrentScenario();

    gGameTexts.Deinit();
}

void CarnageGame::UpdateFrame()
{
    if (!mDebugChangeMapName.empty())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Changing to next map '%s'", mDebugChangeMapName.c_str());
        if (!StartScenario(mDebugChangeMapName))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Fail to change map");
        }
        mDebugChangeMapName.clear();
    }

    float deltaTime = gTimeManager.mGameFrameDelta;

    gSpriteManager.UpdateBlocksAnimations(deltaTime);
    gPhysics.UpdateFrame();
    gGameObjectsManager.UpdateFrame();

    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->UpdateFrame();
    }

    gTrafficManager.UpdateFrame();
    gAiManager.UpdateFrame();
    gBroadcastEvents.UpdateFrame();
}

void CarnageGame::InputEventLost()
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEventLost();
    }
}

void CarnageGame::InputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eKeycode_TILDE)) // show debug console
    {
        gDebugConsoleWindow.mWindowShown = !gDebugConsoleWindow.mWindowShown;
        return;
    }
    if (inputEvent.HasPressed(eKeycode_F3))
    {
        gRenderManager.ReloadRenderPrograms();
        return;
    }

    if (inputEvent.HasPressed(eKeycode_ESCAPE))
    {
        gSystem.QuitRequest();
        return;
    }

    if (inputEvent.HasPressed(eKeycode_C))
    {
        gGameCheatsWindow.mWindowShown = !gGameCheatsWindow.mWindowShown;
        return;
    }

    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
}

void CarnageGame::InputEvent(GamepadInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

bool CarnageGame::SetInputActionsFromConfig()
{
    // force default mapping for first player
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        HumanPlayer* currentPlayer = mHumanPlayers[ihuman];
        if (currentPlayer == nullptr)
            continue;

        currentPlayer->mActionsMapping.Clear();
        if (ihuman == 0) 
        {
            currentPlayer->mActionsMapping.SetDefaults();
        }  
    }

    // open config document
    cxx::json_document configDocument;
    if (!gFiles.ReadConfig(InputsConfigPath, configDocument))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load inputs config from '%s'", InputsConfigPath);
        return false;
    }

    std::string tempString;
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        HumanPlayer* currentPlayer = mHumanPlayers[ihuman];
        if (currentPlayer == nullptr)
            continue;

        tempString = cxx::va("player%d", ihuman + 1);

        cxx::json_document_node rootNode = configDocument.get_root_node();
        cxx::json_document_node configNode = rootNode[tempString];
        currentPlayer->mActionsMapping.LoadConfig(configNode);
    }

    return true;
}

void CarnageGame::SetupHumanPlayer(int humanIndex, Pedestrian* pedestrian)
{
    if (mHumanPlayers[humanIndex])
    {
        debug_assert(false);
        return;
    }

    debug_assert(humanIndex < GAME_MAX_PLAYERS);
    debug_assert(pedestrian);

    if (humanIndex > 0)
    {
        pedestrian->mRemapIndex = humanIndex - 1;
    }

    HumanPlayer* humanPlayer = new HumanPlayer;
    mHumanPlayers[humanIndex] = humanPlayer;

    humanPlayer->mSpawnPosition = pedestrian->GetCurrentPosition();
    humanPlayer->mPlayerView.mFollowCameraController.SetFollowTarget(pedestrian);
    humanPlayer->mPlayerView.mHUD.Setup(pedestrian);
    humanPlayer->SetCharacter(pedestrian);
}

void CarnageGame::DeleteHumanPlayer(int playerIndex)
{
    debug_assert(playerIndex < GAME_MAX_PLAYERS);

    if (mHumanPlayers[playerIndex])
    {
        gRenderManager.DetachRenderView(&mHumanPlayers[playerIndex]->mPlayerView);
        mHumanPlayers[playerIndex]->SetCharacter(nullptr);
        mHumanPlayers[playerIndex]->mPlayerView.SetCameraController(nullptr);

        SafeDelete(mHumanPlayers[playerIndex]);
    }
}

void CarnageGame::SetupScreenLayout()
{   
    const int MaxCols = 2;
    const int playersCount = GetHumanPlayersCount();

    debug_assert(playersCount > 0);

    Rect fullViewport = gGraphicsDevice.mViewportRect;

    int numRows = (playersCount + MaxCols - 1) / MaxCols;
    debug_assert(numRows > 0);

    int frameSizePerH = fullViewport.h / numRows;

    for (int icurr = 0; icurr < playersCount; ++icurr)
    {
        debug_assert(mHumanPlayers[icurr]);

        int currRow = icurr / MaxCols;
        int currCol = icurr % MaxCols;

        int colsOnCurrentRow = glm::clamp(playersCount - (currRow * MaxCols), 1, MaxCols);
        debug_assert(colsOnCurrentRow);
        int frameSizePerW = fullViewport.w / colsOnCurrentRow;
        
        HumanPlayerView& currView = mHumanPlayers[icurr]->mPlayerView;
        currView.mCamera.mViewportRect.h = frameSizePerH;
        currView.mCamera.mViewportRect.x = currCol * (frameSizePerW + 1);
        currView.mCamera.mViewportRect.y = (numRows - currRow - 1) * (frameSizePerH + 1);
        currView.mCamera.mViewportRect.w = frameSizePerW;
 
        currView.SetCameraController(&currView.mFollowCameraController);
        gRenderManager.AttachRenderView(&currView);
    }
}

int CarnageGame::GetHumanPlayerIndex(const HumanPlayer* controller) const
{
    if (controller == nullptr)
    {
        debug_assert(false);
        return -1;
    }
    for (int icurr = 0; icurr < GAME_MAX_PLAYERS; ++icurr)
    {
        if (mHumanPlayers[icurr] == controller)
            return icurr;
    }
    return -1;
}

void CarnageGame::DebugChangeMap(const std::string& mapName)
{
    mDebugChangeMapName = mapName;
}

bool CarnageGame::StartScenario(const std::string& mapName)
{
    ShutdownCurrentScenario();

    if (!gGameMap.LoadFromFile(mapName))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load map '%s'", mapName.c_str());
        return false;
    }
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
    if (!gPhysics.InitPhysicsWorld())
    {
        debug_assert(false);
    }

    gGameObjectsManager.InitGameObjects();

    // temporary
    //glm::vec3 pos { 108.0f, 2.0f, 25.0f };
    //glm::vec3 pos { 14.0, 2.0f, 38.0f };
    //glm::vec3 pos { 91.0f, 2.0f, 236.0f };
    //glm::vec3 pos { 121.0f, 2.0f, 200.0f };
    //glm::vec3 pos { 174.0f, 2.0f, 230.0f };

    int humanPlayersCount = glm::clamp(gSystem.mStartupParams.mPlayersCount, 1, GAME_MAX_PLAYERS);
    gConsole.LogMessage(eLogMessage_Info, "Num players: %d", humanPlayersCount);

    glm::vec3 pos[GAME_MAX_PLAYERS];

    // choose spawn point
    // it is temporary!
    int currFindPosIter = 0;
    for (int yBlock = 10; yBlock < 20 && currFindPosIter < humanPlayersCount; ++yBlock)
    {
        for (int xBlock = 10; xBlock < 20 && currFindPosIter < humanPlayersCount; ++xBlock)
        {
            for (int zBlock = MAP_LAYERS_COUNT - 1; zBlock > -1; --zBlock)
            {
                const MapBlockInfo* currBlock = gGameMap.GetBlockInfo(xBlock, yBlock, zBlock);
                if (currBlock->mGroundType == eGroundType_Field ||
                    currBlock->mGroundType == eGroundType_Pawement ||
                    currBlock->mGroundType == eGroundType_Road)
                {
                    pos[currFindPosIter++] = Convert::MapUnitsToMeters(glm::vec3(xBlock + 0.5f, zBlock, yBlock + 0.5f));
                    break;
                }
            }
        }
    }

    for (int icurr = 0; icurr < humanPlayersCount; ++icurr)
    {
        float randomAngle = 360.0f * gCarnageGame.mGameRand.generate_float();

        Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pos[icurr], cxx::angle_t::from_degrees(randomAngle));
        SetupHumanPlayer(icurr, pedestrian);
    }

    SetInputActionsFromConfig();
    SetupScreenLayout();

    gTrafficManager.StartupTraffic();
    return true;
}

void CarnageGame::ShutdownCurrentScenario()
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        DeleteHumanPlayer(ihuman);
    }
    gAiManager.ReleaseAiControllers();
    gTrafficManager.CleanupTraffic();
    gGameObjectsManager.FreeGameObjects();
    gPhysics.FreePhysicsWorld();
    gGameMap.Cleanup();
    gBroadcastEvents.ClearEvents();
}

int CarnageGame::GetHumanPlayersCount() const
{
    int playersCounter = 0;
    for (HumanPlayer* currPlayer: mHumanPlayers)
    {
        if (currPlayer)
        {
            ++playersCounter;
        }
    }
    return playersCounter;
}
