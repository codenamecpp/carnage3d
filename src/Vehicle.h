#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "GameObject.h"
#include "Sprite2D.h"

//////////////////////////////////////////////////////////////////////////

enum eCarTire
{
    eCarTire_Front,
    eCarTire_Rear,
    eCarTire_COUNT
};

//////////////////////////////////////////////////////////////////////////

// defines vehicle instance
class Vehicle final: public GameObject, public SpriteAnimListener
{
    friend class GameObjectsManager;
    friend class GameCheatsWindow;
    friend class PhysicsManager;

public:
    // public for convenience, should not be modified directly
    VehicleInfo* mCarInfo; // cannot be null
    std::vector<Pedestrian*> mPassengers;

public:
    // @param id: Unique object identifier, constant
    Vehicle(GameObjectID id);

    // override GameObject
    void UpdateFrame() override;
    void SimulationStep() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;
    void HandleDespawn() override;
    void HandleCollision(const Collision& collision) override;
    void HandleCollisionWithMap(const MapCollision& collision) override;
    void HandleFallsOnWater(float fallDistance) override;
    void HandleFallsOnGround(float fallDistance) override;
    bool ReceiveDamage(const DamageInfo& damageInfo) override;
    bool ShouldCollide(GameObject* otherObject) const override;

    // adds or removes car passenger
    // @param pedestrian: Pedestrian, cannot be null
    // @param carSeat: Target seat, does not do check if seat is already occupied, cannot be 'any'
    void RegisterPassenger(Pedestrian* pedestrian, eCarSeat carSeat);
    void UnregisterPassenger(Pedestrian* pedestrian);

    bool HasPassengers() const;

    // get car passengers
    Pedestrian* GetCarDriver() const;
    Pedestrian* GetFirstPassenger(eCarSeat carSeat) const;
    
    // Repair vehicle but only if it not wrecked
    void Repair();

    // doors animations
    void OpenDoor(int doorIndex);
    void CloseDoor(int doorIndex);

    bool HasDoorAnimation(int doorIndex) const;
    bool IsDoorOpened(int doorIndex) const;
    bool IsDoorClosed(int doorIndex) const;
    bool IsDoorOpening(int doorIndex) const;
    bool IsDoorClosing(int doorIndex) const;
    bool HasDoorsOpened() const;

    bool HasHardTop() const;
    bool CanResistElectricity() const;

    // emergency lights animation
    bool HasEmergencyLightsAnimation() const;
    bool IsEmergencyLightsEnabled() const;
    void EnableEmergencyLights(bool isEnabled);

    // get door index for specific seat
    // @param carSeat: Seat identifier, 'any' treated as driver
    int GetDoorIndexForSeat(eCarSeat carSeat) const;

    // get position of specific door in local or world coords
    // local coords is relative to car center
    // @param doorIndex: Door
    // @param out: Position in meters
    bool GetDoorPosLocal(int doorIndex, glm::vec2& out) const;
    bool GetDoorPos(int doorIndex, glm::vec2& out) const;

    // get position of specific seat in local or world coords
    // local coords is relative to car center
    // @param carSeat: Seat
    // @param out: Position in meters
    bool GetSeatPosLocal(eCarSeat carSeat, glm::vec2& out) const;
    bool GetSeatPos(eCarSeat carSeat, glm::vec2& out) const;
    
    bool IsSeatPresent(eCarSeat carSeat) const;

    // Test whether vehicle is wrecked
    bool IsWrecked() const;
    bool IsBurn() const;
    bool IsCriticalDamageState() const;

    // Whether vehicle is fall in water
    bool IsInWater() const;

    // Get current vehicle damage level
    int GetCurrentDamage() const;

    // Get current vehicle speed in meters per hour
    float GetCurrentSpeed() const;

private:
    // override SpriteAnimListener
    bool OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID) override;

    void SetWrecked();
    void Explode();
    void SetupDeltaAnimations();
    void UpdateDeltaAnimations();
    void UpdateEngineSound();

    void SetBurnEffectActive(bool isActive);
    void UpdateBurnEffect();
    void UpdateDamageFromRailways();

    void SetupCarSprite();

    SpriteDeltaBits GetSpriteDeltas() const;

    // physics stuff

    void GetChassisCorners(glm::vec2 corners[4]) const;
    void GetChassisCornersLocal(glm::vec2 corners[4]) const;
    void GetTireCorners(eCarTire tireID, glm::vec2 corners[4]) const;

    struct DriveCtlState
    {
        float mSteerDirection = 0.0f;
        float mDriveDirection = 0.0f;
        bool mHandBrake = false;
    };

    void UpdateSteer(const DriveCtlState& currCtlState);
    void UpdateFriction(const DriveCtlState& currCtlState);
    void UpdateDrive(const DriveCtlState& currCtlState);

    // Get tire velocities
    glm::vec2 GetTireLateralVelocity(eCarTire tireID) const;
    glm::vec2 GetTireForwardVelocity(eCarTire tireID) const;

    // Get tire forward direction and position in world space
    glm::vec2 GetTirePosition(eCarTire tireID) const;
    glm::vec2 GetTireForward(eCarTire tireID) const;
    glm::vec2 GetTireLateral(eCarTire tireID) const;

    glm::vec2 GetTireLocalPos(eCarTire tireID) const;
    glm::vec2 GetTireLocalForward(eCarTire tireID) const;
    glm::vec2 GetTireLocalLateral(eCarTire tireID) const;

private:
    SpriteAnimation mDoorsAnims[MAX_CAR_DOORS];
    SpriteAnimation mEmergLightsAnim;
    SpriteAnimation mDrivingDeltaAnim;
    SpriteDeltaBits mDamageDeltaBits;
    SpriteDeltaBits mPrevDeltaBits;

    // active effects
    Decoration* mFireEffect = nullptr;
    float mBurnStartTime = 0.0f;
    float mStandingOnRailwaysTimer = 0.0f; // how long standing on tracks, seconds
    float mExplosionWaitTime = 0.0f;

    int mRemapIndex = NO_REMAP;
    int mSpriteIndex = 0;
    int mCurrentDamage = 0;

    bool mCarWrecked = false;

    // physics params
    float mFrontTireOffset = 0.0f; // steer
    float mRearTireOffset = 0.0f; // drive
    float mSteeringAngleRadians = 0.0f;
};