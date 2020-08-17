#include "stdafx.h"
#include "GameParams.h"

GameParams gGameParams;

GameParams::GameParams()
{
    SetToDefaults();
}

void GameParams::SetToDefaults()
{
    mPedestrianBoundsSphereRadius = Convert::MapUnitsToMeters(0.10f);
    mPedestrianFistsHitDistance = mPedestrianBoundsSphereRadius + 0.25f;
    mPedestrianTurnSpeed = 260.0f;
    mPedestrianTurnSpeedSlideOnCar = 120.0f;
    mPedestrianSlideOnCarSpeed = Convert::MapUnitsToMeters(1.2f);
    mPedestrianWalkSpeed = Convert::MapUnitsToMeters(0.7f);
    mPedestrianRunSpeed = Convert::MapUnitsToMeters(1.5f);
    mPedestrianSpotTheCarDistance = Convert::MapUnitsToMeters(3.0f);
    mPedestrianKnockedDownTime = 3.0f;
    mPedestrianFallDeathHeight = Convert::MapUnitsToMeters(2.0f);
    mPedestrianDrowningTime = 0.05f;

    // weapons fire rate
    mWeaponsRechargeTime[eWeaponType_Fists] = 1.0f / 2.0f;
    mWeaponsRechargeTime[eWeaponType_Pistol] = 1.0f / 2.5f;
    mWeaponsRechargeTime[eWeaponType_Machinegun] = 1.0f / 8.0f;
    mWeaponsRechargeTime[eWeaponType_Flamethrower] = 1.0f / 14.0f;
    mWeaponsRechargeTime[eWeaponType_RocketLauncher] = 1.0f;
}