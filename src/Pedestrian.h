#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "CharacterController.h"
#include "GameObject.h"
#include "Sprite2D.h"
#include "PedestrianStates.h"
#include "Weapon.h"

// defines generic city pedestrian
class Pedestrian final: public GameObject, public SpriteAnimListener
{
    friend class GameObjectsManager;
    friend class PedestrianPhysics;
    friend class PedestrianStatesManager;
    friend class GameCheatsWindow;

public:
    // public for convenience, should not be modified directly
    ePedestrianType mPedestrianType = ePedestrianType_Civilian;

    CharacterController* mController; // controls pedestrian actions
    
    ePedestrianDeathReason mDeathReason = ePedestrianDeathReason_null; // has meaning only in 'dead state'

    // in car
    Vehicle* mCurrentCar = nullptr;
    eCarSeat mCurrentSeat;

    // inventory
    eWeaponID mCurrentWeapon = eWeapon_Fists;
    eWeaponID mChangeWeapon = eWeapon_Fists;
    Weapon mWeapons[eWeapon_COUNT];

    int mArmorHitPoints = 0;

public:
    // @param id: Unique object identifier, constant
    // @param typeIdentifier: Pedestrian type identifier
    Pedestrian(GameObjectID id, ePedestrianType typeIdentifier);
    ~Pedestrian();

    // override GameObject
    void UpdateFrame() override;
    void SimulationStep() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;
    void HandleDespawn() override;
    bool ShouldCollide(GameObject* otherObject) const override;
    void HandleFallingStarts() override;
    void HandleFallsOnGround(float fallDistance) override;
    void HandleFallsOnWater(float fallDistance) override;
    bool ReceiveDamage(const DamageInfo& damageInfo) override;

    // set next weapon type
    void ChangeWeapon(eWeaponID weapon);

    // Get current weapon state
    // @param weapon: Weapon identifier
    Weapon& GetWeapon();
    Weapon& GetWeapon(eWeaponID weapon);

    void IncArmorMax();
    void IncArmor();
    void DecArmor();
    void ClearAmmunition();

    // Instant kill, pedestrian will remain in dead state until respawn
    void DieFromDamage(const DamageInfo& damageInfo);

    // Process push by other pedestrian
    void PushByPedestrian(Pedestrian* otherPedestrian);

    // Gracefully enter or exit car
    // @param targetCar: Cannot be null
    // @param targetSeat: Cannot be 'any'
    void EnterCar(Vehicle* targetCar, eCarSeat targetSeat);
    void LeaveCar();

    // Force put pedestrian into vehicle or put on the ground 
    void PutInsideCar(Vehicle* car, eCarSeat carSeat);
    void PutOnFoot();

    // check if pedestrian entering/exiting or driving car at this moment
    bool IsCarPassenger() const;
    bool IsCarDriver() const;
    bool IsEnteringOrExitingCar() const;

    // check if pedestrian is in specific state
    bool IsIdle() const; // standing, shooting, walking
    bool IsStanding() const;
    bool IsWalking() const;
    bool IsRunning() const;
    bool IsStunned() const;
    bool IsDead() const;
    bool IsDies() const;
    bool IsBurn() const;
    bool IsOnTheGround() const;

    // Whether pedestrian is fall in water
    bool IsInWater() const;
    
    // Whether pedestrian is under human player control
    bool IsHumanPlayerCharacter() const;
    bool IsAiCharacter() const;

    // Whether pedestrian has specific fears
    bool HasFear_Players() const;
    bool HasFear_Police() const;
    bool HasFear_GunShots() const;
    bool HasFear_Explosions() const;
    bool HasFear_DeadPeds() const;

    ePedestrianAnimID GetCurrentAnimationID() const;

    // Get current pedestrian control state
    const PedestrianCtlState& GetCtlState() const;

private:
    // override SpriteAnimListener
    bool OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID) override;

    bool CanRun() const;

    void SetRemap(int remapIndex);
    void SetAnimation(ePedestrianAnimID animation, eSpriteAnimLoop loopMode);
    void SetDead(ePedestrianDeathReason deathReason);
    void SetCarEntered(Vehicle* targetCar, eCarSeat targetSeat);
    void SetCarExited();
    void SetBurnEffectActive(bool isActive);

    void UpdateBurnEffect();
    void UpdateDamageFromRailways();
    void UpdateDrawOrder();

    void UpdateLocomotion();
    void UpdateRotation();

    // Detects identifier of current pedestrian state
    ePedestrianState GetCurrentStateID() const;

private:
    ePedestrianAnimID mCurrentAnimID;
    SpriteAnimation mCurrentAnimState;
    PedestrianStatesManager mStatesManager;

    float mCurrentStateTime = 0.0f; // time since current state has started
    float mBurnStartTime = 0.0f;
    float mStandingOnRailwaysTimer = 0.0f; // how long standing on tracks, seconds

    bool mContactingOtherPeds = false;
    bool mContactingCars = false;

    int mRemapIndex;

    // active effects
    Decoration* mFireEffect = nullptr;
};

const int Sizeof_Pedestrian = sizeof(Pedestrian);