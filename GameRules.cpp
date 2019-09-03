#include "stdafx.h"
#include "GameRules.h"

GameRules gGameRules;

GameRules::GameRules()
{
    LoadDefaults();
}

void GameRules::LoadDefaults()
{
    mPedestrianTurnSpeed = 30.0f;
    mPedestrianWalkSpeed = MAP_BLOCK_LENGTH * 0.2f;
    mPedestrianRunSpeed = MAP_BLOCK_LENGTH * 0.7f;
    mPedestrianBackWalkSpeed = MAP_BLOCK_LENGTH * 0.05f; 
}