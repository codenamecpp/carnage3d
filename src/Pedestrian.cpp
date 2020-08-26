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

Pedestrian::Pedestrian(GameObjectID id) : GameObject(eGameObjectClass_Pedestrian, id)
    , mPhysicsBody()
    , mCurrentAnimID(ePedestrianAnim_Null)
    , mController()
    , mDrawHeight()
    , mRemapIndex(NO_REMAP)
    , mStatesManager(this)
{
}

Pedestrian::~Pedestrian()
{
    SetCarExited();

    if (mPhysicsBody)
    {
        gPhysics.DestroyPhysicsObject(mPhysicsBody);
    }
}

void Pedestrian::Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation)
{
    mCurrentStateTime = 0.0f;
    mWeaponRechargeTime = 0.0f;
    mBurnStartTime = 0.0f;

    // reset actions
    mCtlState.Clear();

    // reset weapon ammo
    for (int iweapon = 0; iweapon < eWeapon_COUNT; ++iweapon)
    {
        mWeaponsAmmo[iweapon] = -1; // temporary
    }
    mWeaponsAmmo[eWeapon_Fists] = -1;
    mCurrentWeapon = eWeapon_Fists;
    
    if (mPhysicsBody == nullptr)
    {
        mPhysicsBody = gPhysics.CreatePhysicsObject(this, startPosition, startRotation);
        debug_assert(mPhysicsBody);
    }
    else
    {
        mPhysicsBody->SetRespawned();
        mPhysicsBody->SetPosition(startPosition, startRotation);
    }

    mDeathReason = ePedestrianDeathReason_null;

    mCurrentAnimID = ePedestrianAnim_Null;

    PedestrianStateEvent evData { ePedestrianStateEvent_Spawn };
    mStatesManager.ChangeState(ePedestrianState_StandingStill, evData); // force idle state

    SetCarExited();
    SetBurnEffectActive(false);
}

void Pedestrian::UpdateFrame()
{
    // update controller logic if it specified
    if (mController)
    {
        mController->UpdateFrame(this);
    }

    float deltaTime = gTimeManager.mGameFrameDelta;
    mCurrentAnimState.AdvanceAnimation(deltaTime);

    mCurrentStateTime += deltaTime;
    // update current state logic
    mStatesManager.ProcessFrame();

    UpdateBurnEffect();
}

void Pedestrian::PreDrawFrame()
{
    glm::vec3 position = mPhysicsBody->mSmoothPosition;
    ComputeDrawHeight(position);

    cxx::angle_t rotationAngle = mPhysicsBody->GetRotationAngle() ;

    int spriteIndex = mCurrentAnimState.GetCurrentFrame();

    int remapClut = (mRemapIndex == NO_REMAP) ? 0 : mRemapIndex + gGameMap.mStyleData.GetPedestrianRemapsBaseIndex();
    gSpriteManager.GetSpriteTexture(mObjectID, spriteIndex, remapClut, mDrawSprite);

    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mDrawSprite.mRotateAngle = rotationAngle - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    mDrawSprite.mHeight = mDrawHeight;

    // update fire effect draw pos
    // todo: refactore
    if (mFireEffect)
    {
        position.y = mDrawHeight + 0.01f; // todo: magic value
        mFireEffect->SetTransform(position, rotationAngle);
    }
}

void Pedestrian::DrawDebug(DebugRenderer& debugRender)
{
    if (mCurrentCar == nullptr)
    {
        glm::vec3 position = mPhysicsBody->GetPosition();

        WeaponInfo& meleeWeapon = gGameMap.mStyleData.mWeapons[eWeaponFireType_Melee];

        glm::vec2 signVector = mPhysicsBody->GetSignVector() * meleeWeapon.mBaseHitRange;
        debugRender.DrawLine(position, position + glm::vec3(signVector.x, 0.0f, signVector.y), Color32_White, false);

        cxx::bounding_sphere_t bsphere (mPhysicsBody->GetPosition(), gGameParams.mPedestrianBoundsSphereRadius);
        debugRender.DrawSphere(bsphere, Color32_Orange, false);
    }
}

void Pedestrian::ComputeDrawHeight(const glm::vec3& position)
{
    if (mCurrentCar)
    {
        // dont draw pedestrian if it in car with hard top
        if (mCurrentCar->HasHardTop())
        {
            mDrawHeight = mCurrentCar->mDrawHeight - 0.01f; // todo: magic numbers
        }
        else
        {
            mDrawHeight = mCurrentCar->mDrawHeight + 0.01f; // todo: magic numbers
        }
        return;
    }
    
    float maxHeight = position.y;
    if (!mPhysicsBody->mFalling)
    {
        float halfBox = Convert::PixelsToMeters(PED_SPRITE_DRAW_BOX_SIZE_PX) * 0.5f;
        //glm::vec3 points[4] = {
        //    { 0.0f,     position.y + 0.01f, -halfBox },
        //    { halfBox,  position.y + 0.01f, 0.0f },
        //    { 0.0f,     position.y + 0.01f, halfBox },
        //    { -halfBox, position.y + 0.01f, 0.0f },
        //};

        glm::vec3 points[4] = {
            { -halfBox, position.y + 0.01f, -halfBox },
            { halfBox,  position.y + 0.01f, -halfBox },
            { halfBox,  position.y + 0.01f, halfBox },
            { -halfBox, position.y + 0.01f, halfBox },
        };
        for (glm::vec3& currPoint: points)
        {
            //currPoint = glm::rotate(currPoint, angleRadians, glm::vec3(0.0f, -1.0f, 0.0f)); // dont rotate for peds
            currPoint.x += position.x;
            currPoint.z += position.z;

            // get height
            float height = gGameMap.GetHeightAtPosition(currPoint);
            if (height > maxHeight)
            {
                maxHeight = height;
            }
        }
    }

    // todo: get rid of magic numbers
    if (GetCurrentStateID() == ePedestrianState_SlideOnCar)
    {
        maxHeight += 0.35f; // todo: magic numbers
    }

    float drawOffset = 0.02f; // todo: magic numbers
    if (IsUnconscious() || IsDead())
    {
        drawOffset = 0.001f; // todo: magic numbers
    }

    mDrawHeight = maxHeight + drawOffset;
}

void Pedestrian::ChangeWeapon(eWeaponID weapon)
{
    if (mCurrentWeapon == weapon)
        return;

    mCurrentWeapon = weapon;
    // notify current state
    PedestrianStateEvent evData { ePedestrianStateEvent_WeaponChange };
    mStatesManager.ProcessEvent(evData);
}

void Pedestrian::EnterCar(Vehicle* targetCar, eCarSeat targetSeat)
{
    debug_assert(targetSeat != eCarSeat_Any);
    debug_assert(targetCar);

    if (targetCar->IsWrecked())
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

void Pedestrian::ExitCar()
{
    if (IsCarPassenger())
    {
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
    if (mCurrentAnimID != animation)
    {
        mCurrentAnimState.Clear();
        if (!gGameMap.mStyleData.GetPedestrianAnimation(animation, mCurrentAnimState.mAnimDesc))
        {
            debug_assert(false);
        }
        mCurrentAnimID = animation;
    }
    mCurrentAnimState.PlayAnimation(loopMode);
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
    return IsStanding() || IsShooting() || IsWalking();
}

bool Pedestrian::IsStanding() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_StandingStill || currState == ePedestrianState_StandsAndShoots);
}

bool Pedestrian::IsShooting() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_StandsAndShoots || currState == ePedestrianState_WalksAndShoots ||
        currState == ePedestrianState_RunsAndShoots);
}

bool Pedestrian::IsWalking() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_Walks || currState == ePedestrianState_Runs || 
        currState == ePedestrianState_WalksAndShoots || currState == ePedestrianState_RunsAndShoots);
}

bool Pedestrian::IsUnconscious() const
{
    ePedestrianState currState = GetCurrentStateID();
    return currState == ePedestrianState_KnockedDown;
}

bool Pedestrian::IsDead() const
{
    ePedestrianState currState = GetCurrentStateID();
    return currState == ePedestrianState_Dead;
}

void Pedestrian::SetCarEntered(Vehicle* targetCar, eCarSeat targetSeat)
{
    debug_assert(mCurrentCar == nullptr);
    debug_assert(targetCar && targetSeat != eCarSeat_Any);

    mCurrentCar = targetCar;
    mCurrentSeat = targetSeat;

    mCurrentCar->PutPassenger(this, mCurrentSeat);

    // reset actions
    mCtlState.mCtlActions[ePedestrianAction_TurnLeft] = false;
    mCtlState.mCtlActions[ePedestrianAction_TurnRight] = false;
    mCtlState.mCtlActions[ePedestrianAction_Jump] = false;
    mCtlState.mCtlActions[ePedestrianAction_WalkForward] = false;
    mCtlState.mCtlActions[ePedestrianAction_WalkBackward] = false;
    mCtlState.mCtlActions[ePedestrianAction_Run] = false;
    mCtlState.mCtlActions[ePedestrianAction_Shoot] = false;
}

void Pedestrian::SetCarExited()
{
    if (mCurrentCar)
    {
        mCurrentCar->RemovePassenger(this);
        mCurrentCar = nullptr;
    }

    // reset actions
    mCtlState.mCtlActions[ePedestrianAction_HandBrake] = false;
    mCtlState.mCtlActions[ePedestrianAction_Accelerate] = false;
    mCtlState.mCtlActions[ePedestrianAction_Reverse] = false;
    mCtlState.mCtlActions[ePedestrianAction_SteerLeft] = false;
    mCtlState.mCtlActions[ePedestrianAction_SteerRight] = false;
    mCtlState.mCtlActions[ePedestrianAction_Horn] = false;
}

void Pedestrian::SetDead(ePedestrianDeathReason deathReason)
{
    debug_assert(mDeathReason == ePedestrianDeathReason_null);
    debug_assert(deathReason != ePedestrianDeathReason_null);
    mDeathReason = deathReason;

    // notify brains
    if (mController)
    {
        mController->HandleCharacterDeath(this);
    }
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
        case eDamageCause_CarCrash: 
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
        mFireEffect = gGameObjectsManager.CreateDecoration(
            mPhysicsBody->GetPosition(), 
            mPhysicsBody->GetRotationAngle(), &objectInfo);
        debug_assert(mFireEffect);
        if (mFireEffect)
        {
            mFireEffect->SetLifeDuration(0);
            mFireEffect->SetAttachedToObject(this);
        }
        mBurnStartTime = gTimeManager.mGameTime;
    }
    else
    {
        debug_assert(mFireEffect);
        if (mFireEffect)
        {
            mFireEffect->SetDetached();
        }
        mFireEffect->MarkForDeletion();
        mFireEffect = nullptr;
    }
}

void Pedestrian::UpdateBurnEffect()
{
    if (mFireEffect == nullptr)
        return;

    if (IsDead())
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
