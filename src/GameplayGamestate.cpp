#include "stdafx.h"
#include "GameplayGamestate.h"
#include "GameObjectHelpers.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"
#include "SpriteManager.h"
#include "PhysicsManager.h"
#include "WeatherManager.h"
#include "ParticleEffectsManager.h"
#include "TrafficManager.h"
#include "AiManager.h"

void GameplayGamestate::OnGamestateEnter()
{
    // todo
}

void GameplayGamestate::OnGamestateLeave()
{
    // todo
}

void GameplayGamestate::OnGamestateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    gCarnageGame.ProcessDebugCvars();
    // advance game state
    gSpriteManager.UpdateBlocksAnimations(deltaTime);
    gPhysics.UpdateFrame();
    gGameObjectsManager.UpdateFrame();
    gWeatherManager.UpdateFrame();
    gParticleManager.UpdateFrame();
    gTrafficManager.UpdateFrame();
    gAiManager.UpdateFrame();
    gBroadcastEvents.UpdateFrame();
}

void GameplayGamestate::OnGamestateInputEvent(KeyInputEvent& inputEvent)
{
    for (HumanPlayer* currPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currPlayer)
        {
            currPlayer->InputEvent(inputEvent);
        }
    }
}

void GameplayGamestate::OnGamestateInputEvent(MouseButtonInputEvent& inputEvent)
{
    for (HumanPlayer* currPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currPlayer)
        {
            currPlayer->InputEvent(inputEvent);
        }
    }
}

void GameplayGamestate::OnGamestateInputEvent(MouseMovedInputEvent& inputEvent)
{
    for (HumanPlayer* currPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currPlayer)
        {
            currPlayer->InputEvent(inputEvent);
        }
    }
}

void GameplayGamestate::OnGamestateInputEvent(MouseScrollInputEvent& inputEvent)
{
    for (HumanPlayer* currPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currPlayer)
        {
            currPlayer->InputEvent(inputEvent);
        }
    }
}

void GameplayGamestate::OnGamestateInputEvent(KeyCharEvent& inputEvent)
{
    // do nothing
}

void GameplayGamestate::OnGamestateInputEvent(GamepadInputEvent& inputEvent)
{
    for (HumanPlayer* currPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currPlayer)
        {
            currPlayer->InputEvent(inputEvent);
        }
    }
}

void GameplayGamestate::OnGamestateInputEventLost()
{
    for (HumanPlayer* currPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currPlayer)
        {
            currPlayer->InputEventLost();
        }
    }
}

void GameplayGamestate::OnGamestateBroadcastEvent(const BroadcastEvent& broadcastEvent)
{
    if (broadcastEvent.mEventType == eBroadcastEvent_PedestrianDead)
    {
        Pedestrian* pedestrian = ToPedestrian(broadcastEvent.mSubject);
        if (pedestrian && pedestrian->IsHumanPlayerCharacter())
        {
            int playerIndex = gCarnageGame.GetHumanPlayerIndex(pedestrian);
            OnHumanPlayerDie(playerIndex);
        }
        return;
    }

    if (broadcastEvent.mEventType == eBroadcastEvent_StartDriveCar)
    {
        Pedestrian* pedestrian = broadcastEvent.mCharacter;
        if (pedestrian && pedestrian->IsHumanPlayerCharacter())
        {
            int playerIndex = gCarnageGame.GetHumanPlayerIndex(pedestrian);
            OnHumanPlayerStartDriveCar(playerIndex);
        }
        return;
    }
}

void GameplayGamestate::OnHumanPlayerDie(int playerIndex)
{
    debug_assert(playerIndex != -1);

    HumanPlayer* humanPlayer = gCarnageGame.mHumanPlayers[playerIndex];

    debug_assert(humanPlayer);
    debug_assert(humanPlayer->mCharacter);

    humanPlayer->mCharacter->StartGameObjectSound(ePedSfxChannelIndex_Voice, eSfxSampleType_Voice, SfxVoice_PlayerDies, SfxFlags_None);

    // todo: check lives left
    humanPlayer->SetRespawnTimer();

    gConsole.LogMessage(eLogMessage_Info, "Player %d died (%s)", (playerIndex + 1), cxx::enum_to_string(humanPlayer->mCharacter->mDeathReason));

    humanPlayer->mHUD.ShowBigFontMessage(eHUDBigFontMessage_Wasted);
}

void GameplayGamestate::OnHumanPlayerStartDriveCar(int playerIndex)
{
    debug_assert(playerIndex != -1);

    HumanPlayer* humanPlayer = gCarnageGame.mHumanPlayers[playerIndex];

    debug_assert(humanPlayer);
    debug_assert(humanPlayer->mCharacter);

    Vehicle* currentCar = humanPlayer->mCharacter->mCurrentCar;
    debug_assert(currentCar);
    if (currentCar)
    {
        eVehicleModel carModel = currentCar->mCarInfo->mModelID;
        humanPlayer->mHUD.ShowCarNameMessage(carModel);
    }   
}
