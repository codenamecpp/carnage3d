#include "stdafx.h"
#include "GameParams.h"

GameParams gGameParams;

GameParams::GameParams()
{
    SetToDefaults();
}

void GameParams::SetToDefaults()
{
    // todo: move to config

    // pedestrians
    mPedestrianBoundsSphereRadius = Convert::MapUnitsToMeters(0.10f);
    mPedestrianTurnSpeed = 260.0f;
    mPedestrianTurnSpeedSlideOnCar = 120.0f;
    mPedestrianSlideOnCarSpeed = Convert::MapUnitsToMeters(1.2f);
    mPedestrianWalkSpeed = Convert::MapUnitsToMeters(0.7f);
    mPedestrianRunSpeed = Convert::MapUnitsToMeters(1.5f);
    mPedestrianSpotTheCarDistance = Convert::MapUnitsToMeters(3.0f);
    mPedestrianKnockedDownTime = 3.0f;
    mPedestrianFallDeathHeight = Convert::MapUnitsToMeters(2.0f);
    mPedestrianDrowningTime = 0.05f;
    mPedestrianBurnDuration = 4.0f;
    // etc
    mExplosionPrimaryDamageDistance = Convert::MapUnitsToMeters(0.7f);
    mExplosionSecondaryDamageDistance = Convert::MapUnitsToMeters(1.0f);
    mExplosionPrimaryDamage = 100;
    mExplosionSecondaryDamage = 20;

    // vehicles
    mVehicleBurnDuration = 20.0f;
}