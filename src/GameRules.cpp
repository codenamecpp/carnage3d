#include "stdafx.h"
#include "GameRules.h"

GameRules gGameRules;

GameRules::GameRules()
{
    LoadDefaults();
}

void GameRules::LoadDefaults()
{
    mPedestrianTurnSpeed = 260.0f;
    mPedestrianTurnSpeedSlideOnCar = 120.0f;
    mPedestrianSlideOnCarSpeed = MAP_BLOCK_LENGTH * 1.2f;
    mPedestrianWalkSpeed = MAP_BLOCK_LENGTH * 0.7f;
    mPedestrianRunSpeed = MAP_BLOCK_LENGTH * 1.5f;
    mPedestrianSpotTheCarDistance = MAP_BLOCK_LENGTH * 3.0f;
}