#include "stdafx.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "SpriteManager.h"
#include "RenderingManager.h"
#include "PedestrianStates.h"
#include "Vehicle.h"
#include "TimeManager.h"
#include "GameObjectsManager.h"
#include "CarnageGame.h"
#include "Collision.h"
#include "GameObjectHelpers.h"

Pedestrian::Pedestrian(GameObjectID id, ePedestrianType typeIdentifier) 
    : GameObject(eGameObjectClass_Pedestrian, id)
    , mCurrentAnimID(ePedestrianAnim_Null)
    , mController()
    , mRemapIndex(NO_REMAP)
    , mStatesManager(this)
    , mPedestrianTypeID(typeIdentifier)
{
    debug_assert(mPedestrianTypeID < ePedestrianType_COUNT);
    mCurrentAnimState.SetListener(this);
}

Pedestrian::~Pedestrian()
{
    if (mController)
    {
        mController->DeactivateController();
        mController = nullptr;
    }
}

void Pedestrian::HandleSpawn()
{
    debug_assert(mPedestrianTypeID < ePedestrianType_COUNT);
    PedestrianInfo& pedestrianInfo = gGameMap.mStyleData.mPedestrianTypes[mPedestrianTypeID];

    mFearFlags = pedestrianInfo.mFearFlags;

    if (pedestrianInfo.mRemapType == ePedestrianRemapType_Index)
    {
        mRemapIndex = pedestrianInfo.mRemapIndex;
    }
    if (pedestrianInfo.mRemapType == ePedestrianRemapType_RandomCivilian)
    {
        // todo: find out correct civilian peds indices
        mRemapIndex = gCarnageGame.mGameRand.generate_int(0, MAX_PED_REMAPS - 1);
    }

    mCurrentStateTime = 0.0f;

    for (int currWeapon = 0; currWeapon < eWeapon_COUNT; ++currWeapon)
    {
        mWeapons[currWeapon].Setup((eWeaponID) currWeapon, 0);
    }

    mBurnStartTime = 0.0f;
    mStandingOnRailwaysTimer = 0.0f;

    // reset actions
    mCtlState.Clear();

    mCurrentWeapon = eWeapon_Fists;
    mChangeWeapon = eWeapon_Fists;
    
    // create physical body
    CollisionShape collisionShape;
    collisionShape.SetAsCircle(gGameParams.mPedestrianBoundsSphereRadius);
    PhysicsMaterial physicsMaterial;
    physicsMaterial.mDensity = 0.3f; // todo: magic numbers
    PhysicsBody* physicsBody = gPhysics.CreateBody(this, collisionShape, physicsMaterial, 
        CollisionGroup_Pedestrian, CollisionGroup_All, 
        ColliderFlags_None, 
        PhysicsBodyFlags_FixRotation);
    debug_assert(physicsBody);
    SetPhysics(physicsBody);

    mDeathReason = ePedestrianDeathReason_null;
    mCurrentAnimID = ePedestrianAnim_Null;

    PedestrianStateEvent evData { ePedestrianStateEvent_Spawn };
    mStatesManager.ChangeState(ePedestrianState_StandingStill, evData); // force idle state
}

void Pedestrian::HandleDespawn()
{
    SetBurnEffectActive(false);
    SetCarExited();

    // reset weapon
    ClearAmmunition();

    GameObject::HandleDespawn();
}

bool Pedestrian::ShouldCollide(GameObject* otherObject) const
{
    return false;
}

void Pedestrian::HandleFallingStarts()
{
    debug_assert(mPhysicsBody);

    glm::vec2 velocity = mPhysicsBody->GetLinearVelocity();
    if (glm::length2(velocity) > 0.0f)
    {
        velocity = glm::normalize(velocity);
        debug_assert(!glm::isnan(velocity.x) && !glm::isnan(velocity.y));
    }
    mPhysicsBody->ClearForces();
    mPhysicsBody->SetLinearVelocity(velocity);

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightStart };
    mStatesManager.ProcessEvent(evData);
}

void Pedestrian::HandleFallsOnGround(float fallDistance)
{
    debug_assert(mPhysicsBody);

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightEnd };
    mStatesManager.ProcessEvent(evData);

    // damage
    if (fallDistance > 0.0f)
    {
        DamageInfo damageInfo;
        damageInfo.SetDamageFromFall(fallDistance);
        ReceiveDamage(damageInfo);
    }
}

void Pedestrian::HandleFallsOnWater(float fallDistance)
{
    debug_assert(mPhysicsBody);

    PedestrianStateEvent evData { ePedestrianStateEvent_WaterContact };
    mStatesManager.ProcessEvent(evData);
}

void Pedestrian::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mCurrentAnimState.UpdateFrame(deltaTime))
    {
        SetupAnimFrameSprite();
    }

    // update weapons state
    for (Weapon& currWeapon: mWeapons)
    {
        currWeapon.UpdateFrame();
    }

    // change weapon
    if (mCurrentWeapon != mChangeWeapon)
    {
        if (GetWeapon().IsOutOfAmmunition() || GetWeapon().IsReadyToFire())
        {
            mCurrentWeapon = mChangeWeapon;
            // notify current state
            PedestrianStateEvent evData { ePedestrianStateEvent_WeaponChange };
            mStatesManager.ProcessEvent(evData);
        }
    }

    mCurrentStateTime += deltaTime;

    UpdateDamageFromRailways();

    // update current state logic
    mStatesManager.ProcessFrame();
    
    UpdateBurnEffect();
    UpdateDrawOrder();
}

void Pedestrian::SimulationStep()
{
    mContactingOtherPeds = false;
    mContactingCars = false;

    mStatesManager.ProcessSimulationFrame();

    Vehicle* hitCarObject = nullptr;
    float maxCarSpeed = 0.0f;

    // inspect current contacts
    for (const Contact& currContact: mObjectsContacts)
    {
        if (Pedestrian* otherPedestrian = ToPedestrian(currContact.mThatObject))
        {
            mContactingOtherPeds = otherPedestrian->IsOnTheGround() && 
                !otherPedestrian->IsDead() && 
                !otherPedestrian->IsStunned() && 
                !otherPedestrian->IsDies(); // slowdown
        }

        // check car hit
        if (Vehicle* contactCar = ToVehicle(currContact.mThatObject))
        {
            mContactingCars = true;
            if (currContact.mContactPoints->mSeparation > 0.0f)
                continue;

            float penetration = fabsf(currContact.mContactPoints->mSeparation);
            if (penetration < gGameParams.mPedestrianBoundsSphereRadius * 0.9f) // todo: magic numbers
                continue;

            float carSpeed = fabsf(contactCar->GetCurrentSpeed());
            if (carSpeed < maxCarSpeed)
                continue;

            hitCarObject = contactCar;
            maxCarSpeed = carSpeed;
        }
    }

    if (hitCarObject)
    {
        DamageInfo damageInfo;
        damageInfo.SetDamageFromCarHit(hitCarObject);
        ReceiveDamage(damageInfo);
    }

    UpdateLocomotion();
}

void Pedestrian::UpdateDrawOrder()
{
    eSpriteDrawOrder newDrawOrder = eSpriteDrawOrder_Pedestrian;

    if (mCurrentCar)
    {
        newDrawOrder = eSpriteDrawOrder_CarPassenger;
        if (!mCurrentCar->HasHardTop())
        {
            newDrawOrder = eSpriteDrawOrder_ConvetibleCarPassenger;
        }
    }
    else
    {
        if (GetCurrentStateID() == ePedestrianState_SlideOnCar)
        {
            newDrawOrder = eSpriteDrawOrder_JumpingPedestrian;
        }

        if (IsStunned() || IsDead())
        {
            newDrawOrder = eSpriteDrawOrder_Corpse;
        }
    }

    if (mFireEffect)
    {
        mFireEffect->SetDrawOrder(newDrawOrder);
    }

    mDrawSprite.mDrawOrder = newDrawOrder;
}

void Pedestrian::UpdateLocomotion()
{
    if (mPhysicsBody == nullptr)
        return;

    float walkingSpeed = 0.0f;

    bool inverseDirection = false;
    bool isSlideOverCar = (ePedestrianState_SlideOnCar == GetCurrentStateID());

    if (isSlideOverCar)
    {
        walkingSpeed = gGameParams.mPedestrianSlideOnCarSpeed;
    }
    else if (IsIdle())
    {
        // generic case
        if (mCtlState.mWalkForward || mCtlState.mWalkBackward || mCtlState.mRun)
        {
            if (mCtlState.mRun && CanRun())
            {
                walkingSpeed = gGameParams.mPedestrianRunSpeed;
            }
            else
            {
                walkingSpeed = gGameParams.mPedestrianWalkSpeed;

                inverseDirection = mCtlState.mWalkBackward;
            }
        }
    }

    glm::vec2 desiredVelocity (0.0f, 0.0f);
    if (walkingSpeed)
    {
        glm::vec2 moveDirection = mPhysicsBody->GetSignVector() * (inverseDirection ? -1.0f : 1.0f);
        desiredVelocity = moveDirection * walkingSpeed;
        
        if (!isSlideOverCar)
        {
            // prevent walking through cars
            for (const Contact& currContact: mObjectsContacts)
            {
                if (!currContact.mThatObject->IsVehicleClass())
                    continue;

                const ContactPoint& cp = currContact.mContactPoints[0];
                if (glm::dot(-cp.mNormal, moveDirection) > 0.0f)
                {
                    desiredVelocity = glm::vec2(0.0f, 0.0f);
                }
            }
        }
    }

    mPhysicsBody->SetLinearVelocity(desiredVelocity);
}

void Pedestrian::DebugDraw(DebugRenderer& debugRender)
{
    if (mCurrentCar == nullptr)
    {
        glm::vec3 position = mPhysicsBody->GetPosition();

        WeaponInfo& meleeWeapon = gGameMap.mStyleData.mWeaponTypes[eWeaponFireType_Melee];

        glm::vec2 signVector = mPhysicsBody->GetSignVector();
        debugRender.DrawLine(position, position + 
            glm::vec3(signVector.x * meleeWeapon.mBaseHitRange, 0.0f, signVector.y * meleeWeapon.mBaseHitRange), Color32_White, false);

        cxx::bounding_sphere_t bsphere (mPhysicsBody->GetPosition(), gGameParams.mPedestrianBoundsSphereRadius);
        debugRender.DrawSphere(bsphere, Color32_Orange, false);
    }
}

Weapon& Pedestrian::GetWeapon()
{
    return mWeapons[mCurrentWeapon];
}

Weapon& Pedestrian::GetWeapon(eWeaponID weapon)
{
    debug_assert(weapon < eWeapon_COUNT);
    return mWeapons[weapon];
}

void Pedestrian::ChangeWeapon(eWeaponID weapon)
{
    mChangeWeapon = weapon;
}

void Pedestrian::EnterCar(Vehicle* targetCar, eCarSeat targetSeat)
{
    debug_assert(targetSeat != eCarSeat_Any);
    debug_assert(targetCar);

    if (targetCar->IsWrecked())
        return;

    float currentSpeed = targetCar->GetCurrentSpeed();
    if (currentSpeed >= gGameParams.mCarSpeedPassengerCanEnter)
        return;

    if (IsBurn()) // cannot enter vehicle while burn
        return;

    if (IsIdle())
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_EnterCar };
        evData.mTargetCar = targetCar;
        evData.mTargetSeat = targetSeat;
        mStatesManager.ChangeState(ePedestrianState_EnteringCar, evData);
    }
}

void Pedestrian::LeaveCar()
{
    if (IsCarPassenger())
    {
        float currentSpeed = mCurrentCar->GetCurrentSpeed();
        if (currentSpeed >= gGameParams.mCarSpeedPassengerCanEnter)
            return;

        PedestrianStateEvent evData { ePedestrianStateEvent_ExitCar };
        mStatesManager.ChangeState(ePedestrianState_ExitingCar, evData);
    }
}

bool Pedestrian::ReceiveDamage(const DamageInfo& damageInfo)
{
    PedestrianStateEvent evData { ePedestrianStateEvent_ReceiveDamage };
    evData.mDamageInfo = damageInfo;
    return mStatesManager.ProcessEvent(evData);
}

void Pedestrian::SetAnimation(ePedestrianAnimID animation, eSpriteAnimLoop loopMode)
{
    if (mCurrentAnimID == animation)
    {
        if (mCurrentAnimState.IsActive())
        {
            mCurrentAnimState.SetCurrentLoop(loopMode);
            return;
        }

        mCurrentAnimState.PlayAnimation(loopMode);
        SetupAnimFrameSprite();
        return;
    }

    mCurrentAnimState.Clear();
    if (!gGameMap.mStyleData.GetPedestrianAnimation(animation, mCurrentAnimState.mAnimDesc))
    {
        debug_assert(false);
    }
    mCurrentAnimID = animation;
    mCurrentAnimState.PlayAnimation(loopMode);
    SetupAnimFrameSprite();
}

ePedestrianState Pedestrian::GetCurrentStateID() const
{
    return mStatesManager.GetCurrentStateID();
}

bool Pedestrian::IsCarPassenger() const
{
    ePedestrianState currState = GetCurrentStateID();
    if (currState == ePedestrianState_DrivingCar)
    {
        return true; // includes driver
    }
    return false;
}

bool Pedestrian::IsCarDriver() const
{
    ePedestrianState currState = GetCurrentStateID();
    if (currState == ePedestrianState_DrivingCar)
    {
        return mCurrentSeat == eCarSeat_Driver;
    }
    return false;
}

bool Pedestrian::IsEnteringOrExitingCar() const
{
    ePedestrianState currState = GetCurrentStateID();
    if (currState == ePedestrianState_EnteringCar || currState == ePedestrianState_ExitingCar)
    {
        return true;
    }
    return false;
}

bool Pedestrian::IsIdle() const
{
    return IsStanding() || IsWalking();
}

bool Pedestrian::IsStanding() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_StandingStill);
}

bool Pedestrian::IsWalking() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_Walks || currState == ePedestrianState_Runs);
}

bool Pedestrian::IsRunning() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_Runs);
}

bool Pedestrian::IsStunned() const
{
    ePedestrianState currState = GetCurrentStateID();
    return currState == ePedestrianState_Stunned;
}

bool Pedestrian::IsDead() const
{
    ePedestrianState currState = GetCurrentStateID();
    return currState == ePedestrianState_Dead;
}

bool Pedestrian::IsDies() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_Electrocuted);
}

void Pedestrian::SetCarEntered(Vehicle* targetCar, eCarSeat targetSeat)
{
    debug_assert(mCurrentCar == nullptr);
    debug_assert(targetCar && targetSeat != eCarSeat_Any);

    mCurrentCar = targetCar;
    mCurrentSeat = targetSeat;
    mPhysicsBody->ClearForces();
    mCurrentCar->RegisterPassenger(this, mCurrentSeat);
}

void Pedestrian::SetCarExited()
{
    if (mCurrentCar == nullptr)
        return;

    mCurrentCar->UnregisterPassenger(this);
    mCurrentCar = nullptr;
}

void Pedestrian::SetDead(ePedestrianDeathReason deathReason)
{
    debug_assert(mDeathReason == ePedestrianDeathReason_null);
    debug_assert(deathReason != ePedestrianDeathReason_null);
    mDeathReason = deathReason;
}

void Pedestrian::DieFromDamage(eDamageCause damageCause)
{
    PedestrianStateEvent evData { ePedestrianStateEvent_Die };
    evData.mDeathReason = ePedestrianDeathReason_Unknown;

    switch (damageCause)
    {
        case eDamageCause_Gravity: 
            evData.mDeathReason = ePedestrianDeathReason_FallFromHeight;
        break;
        case eDamageCause_Electricity: 
            evData.mDeathReason = ePedestrianDeathReason_Electrocuted;
        break;
        case eDamageCause_Burning: 
            evData.mDeathReason = ePedestrianDeathReason_Fried;
        break;
        case eDamageCause_Drowning: 
            evData.mDeathReason = ePedestrianDeathReason_Drowned;
        break;
        case eDamageCause_CarHit: 
            evData.mDeathReason = ePedestrianDeathReason_Smashed;
        break;
        case eDamageCause_Explosion: 
            evData.mDeathReason = ePedestrianDeathReason_BlownUp;
        break;
        case eDamageCause_Bullet: 
            evData.mDeathReason = ePedestrianDeathReason_Shot;
        break;
        case eDamageCause_Punch: 
            evData.mDeathReason = ePedestrianDeathReason_Beaten;
        break;
    }

    mStatesManager.ChangeState(ePedestrianState_Dead, evData);
}

void Pedestrian::SetBurnEffectActive(bool activate)
{
    if (activate == IsBurn())
        return;

    if (activate)
    {
        debug_assert(mFireEffect == nullptr);
        GameObjectInfo& objectInfo = gGameMap.mStyleData.mObjects[GameObjectType_LFire];

        mFireEffect = gGameObjectsManager.CreateDecoration(mTransform.mPosition, mTransform.mOrientation, &objectInfo);
        debug_assert(mFireEffect);
        if (mFireEffect)
        {
            mFireEffect->SetLifeDuration(0);
            AttachObject(mFireEffect);
        }
        mBurnStartTime = gTimeManager.mGameTime;
    }
    else
    {
        debug_assert(mFireEffect);
        if (mFireEffect)
        {   
            DetachObject(mFireEffect);
        }
        mFireEffect->MarkForDeletion();
        mFireEffect = nullptr;
    }
}

void Pedestrian::UpdateBurnEffect()
{
    if (mFireEffect == nullptr)
        return;

    if (IsDead() || !IsOnTheGround())
        return;

    if (gTimeManager.mGameTime > (mBurnStartTime + gGameParams.mPedestrianBurnDuration))
    {
        DieFromDamage(eDamageCause_Burning);
        return;
    }
}

bool Pedestrian::IsBurn() const
{
    return (mFireEffect != nullptr);
}

bool Pedestrian::IsOnTheGround() const
{
    return !mPhysicsBody->mFalling;
}

bool Pedestrian::IsInWater() const
{
    return mPhysicsBody->mWaterContact;
}

void Pedestrian::PutInsideCar(Vehicle* car, eCarSeat carSeat)
{
    debug_assert(car);
    debug_assert(carSeat < eCarSeat_Any);

    if ((car == nullptr) || (mCurrentCar == car))
        return;

    if (mCurrentCar)
    {
        SetCarExited();
    }

    SetCarEntered(car, carSeat);

    if (!IsDead())
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_None };
        mStatesManager.ChangeState(ePedestrianState_DrivingCar, evData);
    }
}

void Pedestrian::PutOnFoot()
{
    if (mCurrentCar == nullptr)
        return;

    if (!IsDead())
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_None };
        mStatesManager.ChangeState(ePedestrianState_StandingStill, evData);
    }
    SetCarExited();
}

bool Pedestrian::IsHumanPlayerCharacter() const
{
    if (mController)
    {
        return mController->IsHumanPlayer();
    }

    return false;
}

void Pedestrian::ClearAmmunition()
{
    for (int icurrent = 0; icurrent < eWeapon_COUNT; ++icurrent)
    {
        mWeapons[icurrent].SetAmmunition(0);
    }
    mArmorHitPoints = 0;
}

void Pedestrian::UpdateDamageFromRailways()
{
    if (IsDead() || IsDies())
        return;

    if (!IsOnTheGround())
    {
        mStandingOnRailwaysTimer = 0.0f;
        return;
    }

    glm::ivec3 logPosition = Convert::MetersToMapUnits(mTransform.mPosition);

    const MapBlockInfo* blockInfo = gGameMap.GetBlockInfo(logPosition.x, logPosition.z, logPosition.y);
    if ((blockInfo->mGroundType == eGroundType_Field) && blockInfo->mIsRailway)
    {
        mStandingOnRailwaysTimer += gTimeManager.mGameFrameDelta;
        if (mStandingOnRailwaysTimer > gGameParams.mGameRailwaysDamageDelay)
        {
            DamageInfo damageInfo;
            damageInfo.SetDamageFromElectricity();
            ReceiveDamage(damageInfo);
        }
    }
    else
    {
        mStandingOnRailwaysTimer = 0.0f;
    }
}

bool Pedestrian::OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID)
{
    debug_assert(&mCurrentAnimState == animation);

    ePedestrianState stateID = GetCurrentStateID();
    if ((actionID == eSpriteAnimAction_Footstep) && IsHumanPlayerCharacter())
    {
        if ((stateID == ePedestrianState_Runs) || (stateID == ePedestrianState_Walks))
        {
            SfxSampleIndex sfxIndex = (stateID == ePedestrianState_Runs) ? SfxLevel_FootStep2 : SfxLevel_FootStep1;
            StartGameObjectSound(ePedSfxChannelIndex_Misc, eSfxSampleType_Level, sfxIndex, SfxFlags_None);
        }
    }

    return true;
}

ePedestrianAnimID Pedestrian::GetCurrentAnimationID() const
{
    return mCurrentAnimID;
}

void Pedestrian::PushByPedestrian(Pedestrian* otherPedestrian)
{
    debug_assert(otherPedestrian);
    debug_assert(this != otherPedestrian);
    if ((otherPedestrian == nullptr) || (mPhysicsBody == nullptr))
        return;

    // todo: implement
}

void Pedestrian::SetupAnimFrameSprite()
{
    int spriteIndex = mCurrentAnimState.GetSpriteIndex();

    int remapClut = (mRemapIndex == NO_REMAP) ? 0 : mRemapIndex + gGameMap.mStyleData.GetPedestrianRemapsBaseIndex();
    gSpriteManager.GetSpriteTexture(mObjectID, spriteIndex, remapClut, mDrawSprite);
    RefreshDrawSprite();
}

bool Pedestrian::CanRun() const
{
    return !(mContactingOtherPeds && IsHumanPlayerCharacter());
}
