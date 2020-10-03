#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "CharacterController.h"
#include "GameObject.h"
#include "Sprite2D.h"
#include "PedestrianStates.h"
#include "Weapon.h"
#include "PedestrianInfo.h"

// defines generic city pedestrian
class Pedestrian final: public GameObject
    , public SpriteAnimListener
{
    friend class GameObjectsManager;
    friend class PedPhysicsBody;
    friend class PedestrianStatesManager;
    friend class GameCheatsWindow;

public:
    // public for convenience, should not be modified directly
    ePedestrianType mPedestrianTypeID = ePedestrianType_Civilian;

    CharacterController* mController; // controls pedestrian actions
    PedPhysicsBody* mPhysicsBody;

    PedestrianCtlState mCtlState;

    float mDrawHeight;
    int mRemapIndex;
    
    ePedestrianDeathReason mDeathReason = ePedestrianDeathReason_null; // has meaning only in 'dead state'

    // in car
    Vehicle* mCurrentCar = nullptr;
    eCarSeat mCurrentSeat;

    // properties
    ePedestrianFearFlags mFearFlags = ePedestrianFearFlags_None;

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
    void PreDrawFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void Spawn(const glm::vec3& position, cxx::angle_t heading) override;

    // Process damage, it may be ignored depending on type of damage and objects current state
    // @param damageInfo: Damage details
    bool ReceiveDamage(const DamageInfo& damageInfo) override;

    // Current world position
    glm::vec3 GetPosition() const override;
    glm::vec2 GetPosition2() const override;

    // set next weapon type
    void ChangeWeapon(eWeaponID weapon);

    // Get current weapon state
    // @param weapon: Weapon identifier
    Weapon& GetWeapon();
    Weapon& GetWeapon(eWeaponID weapon);

    void ClearAmmunition();

    // Instant kill, pedestrian will remain in dead state until respawn
    void DieFromDamage(eDamageCause damageCause);

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
    bool IsShooting() const;
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

    // Whether pedestrian has specific fears
    bool HasFear_Players() const
    {
        return (mFearFlags & ePedestrianFearFlags_Players) > 0;
    }
    bool HasFear_Police() const
    {
        return (mFearFlags & ePedestrianFearFlags_Police) > 0;
    }
    bool HasFear_GunShots() const
    {
        return (mFearFlags & ePedestrianFearFlags_GunShots) > 0;
    }
    bool HasFear_Explosions() const
    {
        return (mFearFlags & ePedestrianFearFlags_Explosions) > 0;
    }
    bool HasFear_DeadPeds() const
    {
        return (mFearFlags & ePedestrianFearFlags_DeadPeds) > 0;
    }

    ePedestrianAnimID GetCurrentAnimationID() const;

private:
    // override SpriteAnimListener
    bool OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID) override;

    void SetAnimation(ePedestrianAnimID animation, eSpriteAnimLoop loopMode);
    void ComputeDrawHeight(const glm::vec3& position);

    void SetDead(ePedestrianDeathReason deathReason);

    void SetCarEntered(Vehicle* targetCar, eCarSeat targetSeat);
    void SetCarExited();

    void SetBurnEffectActive(bool isActive);
    void UpdateBurnEffect();
    void UpdateDamageFromRailways();

    void SetDrawOrder(eSpriteDrawOrder drawOrder);

    // Detects identifier of current pedestrian state
    ePedestrianState GetCurrentStateID() const;

private:
    ePedestrianAnimID mCurrentAnimID;
    SpriteAnimation mCurrentAnimState;
    PedestrianStatesManager mStatesManager;

    float mCurrentStateTime = 0.0f; // time since current state has started
    float mBurnStartTime = 0.0f;
    float mStandingOnRailwaysTimer = 0.0f; // how long standing on tracks, seconds

    // active effects
    Decoration* mFireEffect = nullptr;
};

const int Sizeof_Pedestrian = sizeof(Pedestrian);