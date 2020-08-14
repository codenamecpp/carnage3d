#pragma once

#include "GameDefs.h"

// defines manager that hold all game logic parameters
class GameParams
{
public:
    GameParams();

    // reset all rules to default values
    void SetToDefaults();

public:
    // pedestrians
    float mPedestrianBoundsSphereRadius; // bounding sphere radius, map units

    float mPedestrianTurnSpeed; // degrees per second
    float mPedestrianTurnSpeedSlideOnCar; // degrees per second
    float mPedestrianSlideOnCarSpeed; // in map units per second
    float mPedestrianWalkSpeed; // in map units per second
    float mPedestrianRunSpeed; // in map units per second
    float mPedestrianKnockedDownTime; // knocked down duration after the punch in face, seconds
    float mPedestrianSpotTheCarDistance; // max distance to detect the car, map units
    float mPedestrianFallDeathHeight; // falling distance which causes pedestrian death, map units
    float mPedestrianDrowningTime; // seconds
    float mWeaponsRechargeTime[eWeaponType_COUNT]; // weapons reload time, seconds
    float mWeaponsDistance[eWeaponType_COUNT]; // map units
};

extern GameParams gGameParams;