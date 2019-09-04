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
    mPedestrianWalkSpeed = MAP_BLOCK_LENGTH * 0.7f;
    mPedestrianRunSpeed = MAP_BLOCK_LENGTH * 1.0f;
    mPedestrianBackWalkSpeed = MAP_BLOCK_LENGTH * 0.5f; 
}