#include "stdafx.h"
#include "GameParams.h"

GameParams gGameParams;

GameParams::GameParams()
{
    LoadDefaults();
}

void GameParams::LoadDefaults()
{
    mPedestrianTurnSpeed = 260.0f;
    mPedestrianTurnSpeedSlideOnCar = 120.0f;
    mPedestrianSlideOnCarSpeed = MAP_BLOCK_LENGTH * 1.2f;
    mPedestrianWalkSpeed = MAP_BLOCK_LENGTH * 0.7f;
    mPedestrianRunSpeed = MAP_BLOCK_LENGTH * 1.5f;
    mPedestrianSpotTheCarDistance = MAP_BLOCK_LENGTH * 3.0f;
    mPedestrianKnockedDownTime = 3.0f;
    mPedestrianFallDeathHeight = MAP_BLOCK_LENGTH * 2.0f;

    // weapons time todo
    mWeaponsRechargeTime[eWeaponType_Fists] = 1.0f / 2.0f;
    mWeaponsRechargeTime[eWeaponType_Pistol] = 1.0f / 4.0f;
    mWeaponsRechargeTime[eWeaponType_Machinegun] = 1.0f / 4.0f;
    mWeaponsRechargeTime[eWeaponType_Flamethrower] = 1.0f / 4.0f;
    mWeaponsRechargeTime[eWeaponType_RocketLauncher] = 1.0f * 3.0f;

    // weapons distance todo
    mWeaponsDistance[eWeaponType_Fists] = PHYSICS_PED_BOUNDING_SPHERE_RADIUS * 1.6f;
    mWeaponsDistance[eWeaponType_Pistol] = 1.0f;
    mWeaponsDistance[eWeaponType_Machinegun] = 1.0f;
    mWeaponsDistance[eWeaponType_Flamethrower] = 1.0f;
    mWeaponsDistance[eWeaponType_RocketLauncher] = 1.0f;
}