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
#include "cvars.h"
#include "ParticleEffectsManager.h"
#include "WeatherManager.h"

//////////////////////////////////////////////////////////////////////////

static const char* InputsConfigPath = "config/inputs.json";

//////////////////////////////////////////////////////////////////////////
// cvars
//////////////////////////////////////////////////////////////////////////

CvarBoolean gCvarMouseAiming("mouse_aiming", false, "Enable mouse aiming", CvarFlags_Archive | CvarFlags_RequiresAppRestart);

CvarString gCvarMapname("g_mapname", "", "Current map name", CvarFlags_Init);
CvarString gCvarCurrentBaseDir("g_basedir", "", "Current gta data location", CvarFlags_Init);
CvarEnum<eGtaGameVersion> gCvarGameVersion("g_gamever", eGtaGameVersion_Unknown, "Current gta game version", CvarFlags_Init);
CvarString gCvarGameLanguage("g_gamelang", "en", "Current game language", CvarFlags_Init);
CvarInt gCvarNumPlayers("g_numplayers", 1, "Number of players in split screen mode", CvarFlags_Init);

// debug
CvarVoid gCvarDbgDumpSpriteDeltas("dbg_dumpSpriteDeltas", "Dump sprite deltas", CvarFlags_None);
CvarVoid gCvarDbgDumpBlockTextures("dbg_dumpBlocks", "Dump block textures", CvarFlags_None);
CvarVoid gCvarDbgDumpSprites("dbg_dumpSprites", "Dump all sprites", CvarFlags_None);
CvarVoid gCvarDbgDumpCarSprites("dbg_dumpCarSprites", "Dump car sprites", CvarFlags_None);

//////////////////////////////////////////////////////////////////////////

CarnageGame gCarnageGame;

//////////////////////////////////////////////////////////////////////////

bool CarnageGame::Initialize()
{
    debug_assert(mCurrentGamestate == nullptr);

    // init randomizer
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    mGameRand.set_seed((unsigned int) ms.count());

    gGameParams.SetToDefaults();

    if (!DetectGameVersion())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Fail to detect game version");
    }

    ::memset(mHumanPlayers, 0, sizeof(mHumanPlayers));

    gGameCheatsWindow.mWindowShown = true; // show by default

    if (gCvarMapname.mValue.empty())
    {
        // try load first found map
        if (!gFiles.mGameMapsList.empty())
        {
            gCvarMapname.mValue = gFiles.mGameMapsList[0];
        }
    }
    gCvarMapname.ClearModified();

    // init texts
    if (!gCvarGameLanguage.mValue.empty())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Set game language: '%s'", gCvarGameLanguage.mValue.c_str());
    }

    std::string textsFilename = GetTextsLanguageFileName(gCvarGameLanguage.mValue);
    gConsole.LogMessage(eLogMessage_Debug, "Loading game texts from '%s'", textsFilename.c_str());

    gGameTexts.Initialize();
    if (!gGameTexts.LoadTexts(textsFilename))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Fail to load game texts for current language");
        gGameTexts.Deinit();
    }

    // init scenario
    if (!StartScenario(gCvarMapname.mValue))
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

    debug_assert(mCurrentGamestate == nullptr);
}

bool CarnageGame::IsMenuGameState() const
{
    return mCurrentGamestate == &mMainMenuGamestate;
}

bool CarnageGame::IsInGameState() const
{
    return mCurrentGamestate == &mGameplayGamestate;
}

void CarnageGame::UpdateFrame()
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateFrame();
    }
}

void CarnageGame::InputEventLost()
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEventLost();
    }
}

void CarnageGame::InputEvent(KeyInputEvent& inputEvent)
{
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

    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(GamepadInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateInputEvent(inputEvent);
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

    HumanPlayer* humanPlayer = new HumanPlayer();
    mHumanPlayers[humanIndex] = humanPlayer;

    // enable mouse aiming for player 0
    if (humanIndex == 0)
    {
        humanPlayer->SetMouseAiming(gCvarMouseAiming.mValue);
    }

    humanPlayer->StartController(pedestrian);
}

void CarnageGame::DeleteHumanPlayer(int playerIndex)
{
    debug_assert(playerIndex < GAME_MAX_PLAYERS);

    if (mHumanPlayers[playerIndex])
    {
        mHumanPlayers[playerIndex]->StopController();
        SafeDelete(mHumanPlayers[playerIndex]);
    }
}

void CarnageGame::SetupScreenLayout()
{   
    const int MaxCols = 2;
    const int playersCount = GetHumanPlayersCount();

    debug_assert(playersCount > 0);

    Point screenResolution = gGraphicsDevice.mScreenResolution;

    int numRows = (playersCount + MaxCols - 1) / MaxCols;
    debug_assert(numRows > 0);

    int frameSizePerH = screenResolution.y / numRows;

    for (int icurr = 0; icurr < playersCount; ++icurr)
    {
        HumanPlayer* currHuman = mHumanPlayers[icurr];
        debug_assert(currHuman);

        int currRow = icurr / MaxCols;
        int currCol = icurr % MaxCols;

        int colsOnCurrentRow = glm::clamp(playersCount - (currRow * MaxCols), 1, MaxCols);
        debug_assert(colsOnCurrentRow > 0);
        int frameSizePerW = screenResolution.x / colsOnCurrentRow;

        Rect viewportArea { currCol * (frameSizePerW + 1), currRow * frameSizePerH, frameSizePerW, frameSizePerH };
        currHuman->SetScreenViewArea(viewportArea);
        currHuman->SetCurrentCameraMode(ePlayerCameraMode_Follow);
    }
}

int CarnageGame::GetHumanPlayerIndex(Pedestrian* pedestrian) const
{
    debug_assert(pedestrian);
    if ((pedestrian == nullptr) || (pedestrian->mController == nullptr))
        return -1;

    for (int icurr = 0; icurr < GAME_MAX_PLAYERS; ++icurr)
    {
        if (mHumanPlayers[icurr] == pedestrian->mController)
            return icurr;
    }
    return -1;
}

bool CarnageGame::StartScenario(const std::string& mapName)
{
    ShutdownCurrentScenario();

    if (mapName.empty())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Map name is not specified");
        return false;
    }

    if (!gGameMap.LoadFromFile(mapName))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load map '%s'", mapName.c_str());
        return false;
    }
    if (!gAudioManager.PreloadLevelSounds())
    {
        // ignore
    }
    gSpriteManager.Cleanup();
    gRenderManager.mMapRenderer.BuildMapMesh();
    if (!gSpriteManager.InitLevelSprites())
    {
        debug_assert(false);
    }

    gPhysics.EnterWorld();
    gParticleManager.EnterWorld();
    gGameObjectsManager.EnterWorld();
    // temporary
    //glm::vec3 pos { 108.0f, 2.0f, 25.0f };
    //glm::vec3 pos { 14.0, 2.0f, 38.0f };
    //glm::vec3 pos { 91.0f, 2.0f, 236.0f };
    //glm::vec3 pos { 121.0f, 2.0f, 200.0f };
    //glm::vec3 pos { 174.0f, 2.0f, 230.0f };

    gCvarNumPlayers.mValue = glm::clamp(gCvarNumPlayers.mValue, 1, GAME_MAX_PLAYERS);
    gConsole.LogMessage(eLogMessage_Info, "Num players: %d", gCvarNumPlayers.mValue);

    glm::vec3 pos[GAME_MAX_PLAYERS];

    // choose spawn point
    // it is temporary!
    int currFindPosIter = 0;
    for (int yBlock = 10; yBlock < 20 && currFindPosIter < gCvarNumPlayers.mValue; ++yBlock)
    {
        for (int xBlock = 10; xBlock < 20 && currFindPosIter < gCvarNumPlayers.mValue; ++xBlock)
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

    for (int icurr = 0; icurr < gCvarNumPlayers.mValue; ++icurr)
    {
        cxx::angle_t pedestrianHeading { 360.0f * gCarnageGame.mGameRand.generate_float(), cxx::angle_t::units::degrees };

        Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pos[icurr], pedestrianHeading, playerPedTypes[icurr]);
        SetupHumanPlayer(icurr, pedestrian);
    }

    SetInputActionsFromConfig();
    SetupScreenLayout();

    gTrafficManager.StartupTraffic();
    gWeatherManager.EnterWorld();

    SetCurrentGamestate(&mGameplayGamestate);
    return true;
}

void CarnageGame::ShutdownCurrentScenario()
{
    SetCurrentGamestate(nullptr);
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        DeleteHumanPlayer(ihuman);
    }
    gAiManager.ReleaseAiControllers();
    gTrafficManager.CleanupTraffic();
    gWeatherManager.ClearWorld();
    gGameObjectsManager.ClearWorld();
    gPhysics.ClearWorld();
    gGameMap.Cleanup();
    gBroadcastEvents.ClearEvents();
    gAudioManager.ReleaseLevelSounds();
    gParticleManager.ClearWorld();
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
    bool useAutoDetection = (gCvarGameVersion.mValue == eGtaGameVersion_Unknown);
    if (useAutoDetection)
    {
        const int GameMapsCount = (int) gFiles.mGameMapsList.size();
        if (GameMapsCount == 0)
            return false;

        if (gFiles.IsFileExists("MISSUK.INI"))
        {
            gCvarGameVersion.mValue = eGtaGameVersion_MissionPack1_London69;
        }
        else if (gFiles.IsFileExists("missuke.ini"))
        {
            gCvarGameVersion.mValue = eGtaGameVersion_MissionPack2_London61;
        }
        else if (GameMapsCount < 3)
        {
            gCvarGameVersion.mValue = eGtaGameVersion_Demo;
        }
        else
        {
            gCvarGameVersion.mValue = eGtaGameVersion_Full;
        }
    }

    gConsole.LogMessage(eLogMessage_Debug, "Gta game version is '%s' (%s)", cxx::enum_to_string(gCvarGameVersion.mValue),
        useAutoDetection ? "autodetect" : "forced");
    
    return true;
}

std::string CarnageGame::GetTextsLanguageFileName(const std::string& languageID) const
{
    if ((gCvarGameVersion.mValue == eGtaGameVersion_Demo) || (gCvarGameVersion.mValue == eGtaGameVersion_Full))
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

    if (gCvarGameVersion.mValue == eGtaGameVersion_MissionPack1_London69)
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

    if (gCvarGameVersion.mValue == eGtaGameVersion_MissionPack2_London61)
    {
        return "enguke.fxt";
    }
   
    return "ENGLISH.FXT";
}

void CarnageGame::ProcessDebugCvars()
{
    if (gCvarDbgDumpSpriteDeltas.IsModified())
    {
        gCvarDbgDumpSpriteDeltas.ClearModified();
        std::string savePath = gFiles.mExecutableDirectory + "/sprite_deltas";
        gSpriteManager.DumpSpriteDeltas(savePath);
        gConsole.LogMessage(eLogMessage_Info, "Sprite deltas path is '%s'", savePath.c_str());
    }

    if (gCvarDbgDumpBlockTextures.IsModified())
    {
        gCvarDbgDumpBlockTextures.ClearModified();
        std::string savePath = gFiles.mExecutableDirectory + "/block_textures";
        gSpriteManager.DumpBlocksTexture(savePath);
        gConsole.LogMessage(eLogMessage_Info, "Blocks textures path is '%s'", savePath.c_str());
    }

    if (gCvarDbgDumpSprites.IsModified())
    {
        gCvarDbgDumpSprites.ClearModified();
        std::string savePath = gFiles.mExecutableDirectory + "/sprites";
        gSpriteManager.DumpSpriteTextures(savePath);
        gConsole.LogMessage(eLogMessage_Info, "Sprites path is '%s'", savePath.c_str());
    }

    if (gCvarDbgDumpCarSprites.IsModified())
    {
        gCvarDbgDumpCarSprites.ClearModified();
        std::string savePath = gFiles.mExecutableDirectory + "/car_sprites";
        gSpriteManager.DumpCarsTextures(savePath);
        gConsole.LogMessage(eLogMessage_Info, "Car sprites path is '%s'", savePath.c_str());
    }
}

void CarnageGame::SetCurrentGamestate(GenericGamestate* gamestate)
{
    if (mCurrentGamestate == gamestate)
        return;

    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateLeave();
    }

    mCurrentGamestate = gamestate;
    if (mCurrentGamestate)
    {
        mCurrentGamestate->OnGamestateEnter();
    }
}
