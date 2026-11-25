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
    // game - commons
    float mGamePlayerRespawnTime; // respawn time, seconds
    float mGameRailwaysDamageDelay; // how long can to stand on railways without damage, seconds
    float mGameGourangaComboTime; // max time between hare krishnas kills to continue gouranga combo, seconds

    // pedestrians
    float mPedestrianBoundsSphereRadius; // bounding sphere radius, meters
    float mPedestrianTurnSpeed; // degrees per second
    float mPedestrianTurnSpeedSlideOnCar; // degrees per second
    float mPedestrianSlideOnCarSpeed; // meters per second
    float mPedestrianWalkSpeed; // meters per second
    float mPedestrianRunSpeed; // meters per second
    float mPedestrianKnockedDownTime; // knocked down duration after the punch in face, seconds
    float mPedestrianSpotTheCarDistance; // max distance to detect the car, meters
    float mPedestrianFallDeathHeight; // falling distance which causes pedestrian death, meters
    float mPedestrianDrowningTime; // seconds
    float mPedestrianBurnDuration; // time while pedestrian can live while burning, seconds
    int mPedestrianMaxArmor; // max armor points

    // traffic - common params
    int mTrafficGenHareKrishnasPedsInterval; // number of traffic pedestrians to generate before next hare krishnas group
    int mTrafficHareKrishnasGroupSize; // number of pedestrians in hare krishnas group

    // traffic - pedestrians
    int mTrafficGenMaxPeds; // max number of traffic pedestrians around the player
    int mTrafficGenPedsChance; // chance to generate new traffic pedestrian on current turn
    int mTrafficGenPedsMaxDistance; // maximum distance from player camera, blocks
    float mTrafficGenPedsCooldownTime; // seconds between traffic generation

    // traffic - cars
    int mTrafficGenMaxCars; // max number of traffic cars around the player
    int mTrafficGenCarsChance; // chance to generate new traffic car on current turn
    int mTrafficGenCarsMaxDistance; // maximum distance from player camera, blocks
    float mTrafficGenCarsCooldownTime; // seconds between traffic generation

    // explosion
    float mExplosionRadius; // how far explosion can do damage, meters

    // vehicles
    float mCarBurnDuration; // time before flame will go out by itself, seconds
    float mCarSpeedPassengerCanEnter; // passenger can enter or exit vehicle only if speed is low
    float mCarExplosionChainDelayTime; // delay before car explode, seconds
    float mCarFallSpeed; // constant, meters per second
    float mCarFallStartHeight; // vehicle falls only when ground is lower than this, meters
    float mCarFallLookAheadTime; // ground check looks ahead this time, seconds
    float mCarWallRestitution; // part of speed towards wall that vehicle keeps after hit
    float mCarMinImpactSpeed; // slower hits do not bounce and do not damage, meters per second
    int mCarSkidMarksMaxCount; // max skid marks on map, 0 for no limit
    // vehicles misc
    float mTurretRotationSpeed; // tank/firetruck turret, degrees per second

    // broadcast events
    float mBroadcastGunShotEventDuration; // how long event affects, seconds
    float mBroadcastExplosionEventDuration;

    // ai
    float mAiReactOnGunshotsDistance; // how far pedestrians can hear gunshots
    float mAiReactOnExplosionsDistance; // how far pedestrians can hear explosions

    // hud
    float mHudBigFontMessageShowDuration; // how long show 'wasted' on screen, seconds
    float mHudCarNameShowDuration; // how long show car name on screen, seconds
    float mHudDistrictNameShowDuration; // how long show district name on screen, seconds

    // collision
    float mSparksOnCarsContactThreshold; // how strong should the collision be for sparks to appear
};

extern GameParams gGameParams;