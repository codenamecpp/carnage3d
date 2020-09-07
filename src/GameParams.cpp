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
    mPedestrianWalkSpeed = Convert::MapUnitsToMeters(0.5f);
    mPedestrianRunSpeed = Convert::MapUnitsToMeters(1.5f);
    mPedestrianSpotTheCarDistance = Convert::MapUnitsToMeters(3.0f);
    mPedestrianKnockedDownTime = 3.0f;
    mPedestrianFallDeathHeight = Convert::MapUnitsToMeters(2.0f);
    mPedestrianDrowningTime = 0.05f;
    mPedestrianBurnDuration = 4.0f;
    // traffic - pedestrians
    mTrafficGenMaxPeds = 20;
    mTrafficGenPedsChance = 50;
    mTrafficGenPedsMaxDistance = 2;
    mTrafficGenPedsCooldownTime = 1.5f;
    // traffic - cars
    mTrafficGenMaxCars = 12;
    mTrafficGenCarsChance = 65;
    mTrafficGenCarsMaxDistance = 4;
    mTrafficGenCarsMinDistance = 1;
    mTrafficGenCarsCooldownTime = 3.0f;
    // explosion
    mExplosionPrimaryDamageDistance = Convert::MapUnitsToMeters(0.25f);
    mExplosionSecondaryDamageDistance = Convert::MapUnitsToMeters(0.75f);
    mExplosionPrimaryDamage = 100;
    mExplosionSecondaryDamage = 1;
    // vehicles
    mVehicleBurnDuration = 20.0f;
}