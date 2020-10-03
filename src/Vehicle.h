#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "GameObject.h"
#include "Sprite2D.h"

// defines vehicle instance
class Vehicle final: public GameObject
    , public SpriteAnimListener
{
    friend class GameObjectsManager;
    friend class GameCheatsWindow;

public:
    // public for convenience, should not be modified directly
    CarPhysicsBody* mPhysicsBody;

    float mDrawHeight;

    VehicleInfo* mCarInfo; // cannot be null
    std::vector<Pedestrian*> mPassengers;

public:
    // @param id: Unique object identifier, constant
    Vehicle(GameObjectID id);
    ~Vehicle();

    // override GameObject
    void UpdateFrame() override;
    void PreDrawFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void Spawn(const glm::vec3& position, cxx::angle_t heading) override;
    // Process damage, it may be ignored depending on type of damage and objects current state
    // @param damageInfo: Damage details
    bool ReceiveDamage(const DamageInfo& damageInfo) override;

    // Current world position
    glm::vec3 GetPosition() const override;
    glm::vec2 GetPosition2() const override;

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

    // Whether vehicle is fall in water
    bool IsInWater() const;

    // Get current vehicle damage level
    int GetCurrentDamage() const;

private:
    // override SpriteAnimListener
    bool OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID) override;

    void SetWrecked();
    void Explode();
    void ComputeDrawHeight(const glm::vec3& position);
    void SetupDeltaAnimations();
    void UpdateDeltaAnimations();

    void SetBurnEffectActive(bool isActive);
    void UpdateBurnEffect();
    void UpdateDamageFromRailways();

    SpriteDeltaBits GetSpriteDeltas() const;

private:
    SpriteAnimation mDoorsAnims[MAX_CAR_DOORS];
    SpriteAnimation mEmergLightsAnim;
    SpriteAnimation mDrivingDeltaAnim;
    SpriteDeltaBits mDamageDeltaBits;

    // active effects
    Decoration* mFireEffect = nullptr;
    float mBurnStartTime = 0.0f;
    float mStandingOnRailwaysTimer = 0.0f; // how long standing on tracks, seconds

    int mRemapIndex = NO_REMAP;
    int mSpriteIndex = 0;
    int mCurrentDamage = 0;

    bool mCarWrecked = false;
};