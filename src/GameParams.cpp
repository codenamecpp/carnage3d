#include "stdafx.h"
#include "GameParams.h"

GameParams gGameParams;

GameParams::GameParams()
{
    SetToDefaults();
}

void GameParams::SetToDefaults()
{
    mPedestrianBoundsSphereRadius = 0.15f;

    mPedestrianTurnSpeed = 260.0f;
    mPedestrianTurnSpeedSlideOnCar = 120.0f;
    mPedestrianSlideOnCarSpeed = 1.2f;
    mPedestrianWalkSpeed = 0.7f;
    mPedestrianRunSpeed = 1.5f;
    mPedestrianSpotTheCarDistance = 3.0f;
    mPedestrianKnockedDownTime = 3.0f;
    mPedestrianFallDeathHeight = 2.0f;
    mPedestrianDrowningTime = 0.05f;

    // weapons time todo
    mWeaponsRechargeTime[eWeaponType_Fists] = 1.0f / 2.0f;
    mWeaponsRechargeTime[eWeaponType_Pistol] = 1.0f / 4.0f;
    mWeaponsRechargeTime[eWeaponType_Machinegun] = 1.0f / 4.0f;
    mWeaponsRechargeTime[eWeaponType_Flamethrower] = 1.0f / 4.0f;
    mWeaponsRechargeTime[eWeaponType_RocketLauncher] = 1.0f * 3.0f;

    // weapons distance todo
    mWeaponsDistance[eWeaponType_Fists] = mPedestrianBoundsSphereRadius + 0.6f;
    mWeaponsDistance[eWeaponType_Pistol] = 1.0f;
    mWeaponsDistance[eWeaponType_Machinegun] = 1.0f;
    mWeaponsDistance[eWeaponType_Flamethrower] = 1.0f;
    mWeaponsDistance[eWeaponType_RocketLauncher] = 1.0f;
}