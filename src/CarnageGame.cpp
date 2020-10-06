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
#include "AudioManager.h"

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

    if (!DetectGameVersion())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Fail to detect game version");
    }

    // init tests
    std::string gameLanguageID = gSystem.mStartupParams.mGameLanguage;
    if (!gameLanguageID.empty())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Set game language: '%s'", gameLanguageID.c_str());
    }

    std::string textsFilename = GetTextsLanguageFileName(gameLanguageID);
    gConsole.LogMessage(eLogMessage_Debug, "Loading game texts from '%s'", textsFilename.c_str());

    gGameTexts.Initialize();
    if (!gGameTexts.LoadTexts(textsFilename))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Fail to load game texts for current language");
        gGameTexts.Deinit();
    }

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

    HumanPlayer* humanPlayer = new HumanPlayer(pedestrian);
    mHumanPlayers[humanIndex] = humanPlayer;

    humanPlayer->mSpawnPosition = pedestrian->GetPosition();
    humanPlayer->mPlayerView.mFollowCameraController.SetFollowTarget(pedestrian);
    humanPlayer->mPlayerView.mHUD.SetupHUD(humanPlayer);
}

void CarnageGame::DeleteHumanPlayer(int playerIndex)
{
    debug_assert(playerIndex < GAME_MAX_PLAYERS);

    if (mHumanPlayers[playerIndex])
    {
        gRenderManager.DetachRenderView(&mHumanPlayers[playerIndex]->mPlayerView);
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
    if (!gAudioManager.LoadLevelSounds())
    {
        // ignore
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

    ePedestrianType playerPedTypes[GAME_MAX_PLAYERS] =
    {
        ePedestrianType_Player1,
        ePedestrianType_Player2,
        ePedestrianType_Player3,
        ePedestrianType_Player4,
    };

    for (int icurr = 0; icurr < humanPlayersCount; ++icurr)
    {
        cxx::angle_t pedestrianHeading { 360.0f * gCarnageGame.mGameRand.generate_float(), cxx::angle_t::units::degrees };

        Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pos[icurr], pedestrianHeading, playerPedTypes[icurr]);
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
    gAudioManager.FreeLevelSounds();
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

bool CarnageGame::DetectGameVersion()
{
    mGameVersion = eGtaGameVersion_Full;

    bool useAutoDetection = true;

    SystemStartupParams& params = gSystem.mStartupParams;
    if (!params.mGtaGameVersion.empty())
    {
        if (!cxx::parse_enum(params.mGtaGameVersion.c_str(), mGameVersion))
        {
            gConsole.LogMessage(eLogMessage_Debug, "Unknown game version '%s', ignore", params.mGtaGameVersion.c_str());
        }
        else
        {
            useAutoDetection = false;
        }
    }

    if (useAutoDetection)
    {
        const int GameMapsCount = (int) gFiles.mGameMapsList.size();
        if (GameMapsCount == 0)
            return false;

        if (gFiles.IsFileExists("MISSUK.INI"))
        {
            mGameVersion = eGtaGameVersion_MissionPack1_London69;
        }
        else if (gFiles.IsFileExists("missuke.ini"))
        {
            mGameVersion = eGtaGameVersion_MissionPack2_London61;
        }
        else if (GameMapsCount < 3)
        {
            mGameVersion = eGtaGameVersion_Demo;
        }
        else
        {
            mGameVersion = eGtaGameVersion_Full;
        }
    }

    gConsole.LogMessage(eLogMessage_Debug, "Gta game version is '%s' (%s)", cxx::enum_to_string(mGameVersion),
        useAutoDetection ? "autodetect" : "forced");
    
    return true;
}

std::string CarnageGame::GetTextsLanguageFileName(const std::string& languageID) const
{
    if ((mGameVersion == eGtaGameVersion_Demo) || (mGameVersion == eGtaGameVersion_Full))
    {
        if (cxx_stricmp(languageID.c_str(), "en") == 0)
        {
            return "ENGLISH.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "fr") == 0)
        {
            return "FRENCH.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "de") == 0)
        {
            return "GERMAN.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "it") == 0)
        {
            return "ITALIAN.FXT";
        }

        return "ENGLISH.FXT";
    }

    if (mGameVersion == eGtaGameVersion_MissionPack1_London69)
    {
        if (cxx_stricmp(languageID.c_str(), "en") == 0)
        {
            return "ENGUK.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "fr") == 0)
        {
            return "FREUK.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "de") == 0)
        {
            return "GERUK.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "it") == 0)
        {
            return "ITAUK.FXT";
        }

        return "ENGUK.FXT";
    }

    if (mGameVersion == eGtaGameVersion_MissionPack2_London61)
    {
        return "enguke.fxt";
    }

    debug_assert(false);
    return "ENGLISH.FXT";
}
