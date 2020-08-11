#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "GameObject.h"
#include "Sprite2D.h"

class SpriteBatch;
class DebugRenderer;

// defines vehicle instance
class Vehicle final: public GameObject
{
    friend class GameObjectsManager;

public:
    // public for convenience, should not be modified directly
    CarPhysicsComponent* mPhysicsComponent;

    bool mDead;

    float mDrawHeight;
    int mRemapIndex;

    CarStyle* mCarStyle; // cannot be null
    std::vector<Pedestrian*> mPassengers;

public:
    // @param id: Unique object identifier, constant
    Vehicle(GameObjectID id);
    ~Vehicle();

    void UpdateFrame() override;
    void DrawFrame(SpriteBatch& spriteBatch) override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation);

    // todo: implement car damage system
    void ReceiveDamageFromWater();

    // adds passenger into the car
    // @param pedestrian: Pedestrian, cannot be null
    // @param carSeat: Target seat, does not do check if seat is already occupied, cannot be 'any'
    void PutPassenger(Pedestrian* pedestrian, eCarSeat carSeat);

    // removes passenger
    void RemovePassenger(Pedestrian* pedestrian);

    // get car passengers
    Pedestrian* GetCarDriver() const;
    Pedestrian* GetFirstPassenger(eCarSeat carSeat) const;

    // doors animations
    void OpenDoor(int doorIndex);
    void CloseDoor(int doorIndex);

    bool HasDoorAnimation(int doorIndex) const;
    bool IsDoorOpened(int doorIndex) const;
    bool IsDoorClosed(int doorIndex) const;
    bool IsDoorOpening(int doorIndex) const;
    bool IsDoorClosing(int doorIndex) const;

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
    bool GetDoorPosLocal(int doorIndex, glm::vec2& out) const;
    bool GetDoorPos(int doorIndex, glm::vec2& out) const;

    // get position of specific seat in local or world coords
    // local coords is relative to car center
    // @param carSeat: Seat
    bool GetSeatPosLocal(eCarSeat carSeat, glm::vec2& out) const;
    bool GetSeatPos(eCarSeat carSeat, glm::vec2& out) const;
    
    bool IsSeatPresent(eCarSeat carSeat) const;

private:
    void UpdateDriving();
    void ComputeDrawHeight(const glm::vec3& position);
    void SetupDeltaAnimations();
    void UpdateDeltaAnimations();

    SpriteDeltaBits GetSpriteDeltas() const;

private:
    Sprite2D mChassisDrawSprite;

    SpriteAnimation mDoorsAnims[MAX_CAR_DOORS];
    SpriteAnimation mEmergLightsAnim;
    SpriteDeltaBits mDamageDeltaBits;

    int mChassisSpriteIndex = 0;

    // internal stuff that can be touched only by PedestrianManager
    cxx::intrusive_node<Vehicle> mCarsListNode;
};