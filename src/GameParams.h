#pragma once

#include "GameDefs.h"

// defines manager that hold all game logic parameters
class GameParams
{
public:
    GameParams();

    // reset all rules to default values
    void LoadDefaults();

public:
    // pedestrians
    float mPedestrianTurnSpeed; // degrees per second
    float mPedestrianTurnSpeedSlideOnCar; // degrees per second
    float mPedestrianSlideOnCarSpeed; // in blocks per second
    float mPedestrianWalkSpeed; // in blocks per second
    float mPedestrianRunSpeed; // in blocks per second
    float mPedestrianKnockedDownTime; // knocked down duration after the punch in face, seconds
    float mPedestrianSpotTheCarDistance; // max distance to detect the car
    float mPedestrianFallDeathHeight; // falling distance which causes pedestrian death
    float mPedestrianDrowningTime; // specified in seconds
    float mWeaponsRechargeTime[eWeaponType_COUNT]; // weapons reload time specified in seconds
    float mWeaponsDistance[eWeaponType_COUNT]; // specified in blocks
};

extern GameParams gGameParams;