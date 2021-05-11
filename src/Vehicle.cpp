#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsBody.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "Pedestrian.h"
#include "TimeManager.h"
#include "GameObjectsManager.h"
#include "AudioManager.h"
#include "Collider.h"

Vehicle::Vehicle(GameObjectID id) : GameObject(eGameObjectClass_Car, id)
    , mCarWrecked()
    , mCarInfo()
    , mDamageDeltaBits()
    , mDrivingDeltaAnim()
{
}

void Vehicle::HandleSpawn()
{
    mStandingOnRailwaysTimer = 0.0f;

    debug_assert(mCarInfo);

    mRearTireOffset = Convert::PixelsToMeters(mCarInfo->mDriveWheelOffset);
    mFrontTireOffset = Convert::PixelsToMeters(mCarInfo->mSteeringWheelOffset);
    mSteeringAngleRadians = 0.0f;
    
    glm::vec3 halfExtents { 
        mCarInfo->mDimensions.z * 0.5f, 
        mCarInfo->mDimensions.y * 0.5f, 
        mCarInfo->mDimensions.x * 0.5f }; // swap z and x

    CollisionShape shapeData;
    shapeData.SetAsBox(halfExtents, glm::vec3());

    PhysicsMaterial materialData;
    materialData.mDensity = 80.0f; // todo: magic numbers
    materialData.mFriction = 0.0f;
    materialData.mRestitution = 0.0f;

    PhysicsBody* physicsBody = gPhysics.CreateBody(this, shapeData, materialData, CollisionGroup_Car, CollisionGroup_All, ColliderFlags_None, PhysicsBodyFlags_None);
    debug_assert(physicsBody);
    SetPhysics(physicsBody);

    mCarWrecked = false;
    mCurrentDamage = 0;
    mDamageDeltaBits = 0;
    mPrevDeltaBits = 0;
    mSpriteIndex = mCarInfo->mSpriteIndex; // todo: handle bike fallen state 

    SetupDeltaAnimations();
    SetRemap(NO_REMAP);

    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Car;
    SetSprite(mSpriteIndex, GetSpriteDeltas());
}

void Vehicle::HandleDespawn()
{
    SetBurnEffectActive(false);  

    // eject passengers
    if (HasPassengers())
    {
        std::vector<Pedestrian*> passengers = mPassengers;
        for (Pedestrian* currPassenger: passengers)
        {
            if (currPassenger->IsMarkedForDeletion())
                continue;

            currPassenger->PutOnFoot();
        }
        mPassengers.clear();
    }

    // force stop sounds
    StopGameObjectSounds();

    gSpriteManager.FlushSpritesCache(mObjectID);

    GameObject::HandleDespawn();
}

void Vehicle::UpdateFrame()
{
    UpdateBurnEffect();
    UpdateDamageFromRailways();
    UpdateEngineSound();
    if (IsWrecked())
        return;

    // check if car destroyed
    if (IsCriticalDamageState())
    {
        if (mExplosionWaitTime > 0.0f)
        {
            mExplosionWaitTime -= gTimeManager.mGameFrameDelta;
            if (mExplosionWaitTime > 0.0f)
                return;
        }
        SetWrecked();
        Explode();
        return;
    }

    UpdateDeltaAnimations();
}

void Vehicle::SimulationStep()
{
    DriveCtlState currCtlState;

    if (!IsWrecked())
    {
        Pedestrian* carDriver = GetCarDriver();
        if (carDriver)
        {
            currCtlState.mDriveDirection = carDriver->mCtlState.mAcceleration;
            currCtlState.mSteerDirection = carDriver->mCtlState.mSteerDirection;
            currCtlState.mHandBrake = carDriver->mCtlState.mHandBrake;
        }
    }
    UpdateFriction(currCtlState);
    UpdateDrive(currCtlState);
    UpdateSteer(currCtlState);
}

void Vehicle::DebugDraw(DebugRenderer& debugRender)
{
    glm::vec3 position = mPhysicsBody->GetPosition();

    glm::vec2 corners[4];
    GetChassisCorners(corners);

    float drawHeight = mPhysicsBody->mPositionY;

    glm::vec3 points[4];
    for (int i = 0; i < 4; ++i)
    {
        points[i].x = corners[i].x;
        points[i].z = corners[i].y;
        points[i].y = drawHeight;
    }
    for (int i = 0; i < 4; ++i)
    {
        debugRender.DrawLine(points[i], points[(i + 1) % 4], Color32_Red, false);
    }

    // draw doors
    for (int idoor = 0; idoor < mCarInfo->mDoorsCount; ++idoor)
    {
        glm::vec2 rotated_pos;
        GetDoorPos(idoor, rotated_pos);
        debugRender.DrawCube(glm::vec3(rotated_pos.x, position.y + 0.15f, rotated_pos.y), glm::vec3(0.15f, 0.15f, 0.15f), Color32_Yellow, false);
    }

    if (mCarInfo->mDoorsCount > 0)
    {
        glm::vec2 seatpos;
        GetSeatPos(eCarSeat_Driver, seatpos);
        debugRender.DrawCube(glm::vec3(seatpos.x, position.y + 0.15f, seatpos.y), glm::vec3(0.15f, 0.15f, 0.15f), Color32_Green, false);
    }

    // draw body velocity
    glm::vec2 bodyLinearVelocity = mPhysicsBody->GetLinearVelocity();
    debugRender.DrawLine(
        glm::vec3 {position.x, drawHeight, position.z},
        glm::vec3 {position.x + bodyLinearVelocity.x, drawHeight, position.z + bodyLinearVelocity.y}, Color32_Cyan, false);

    // draw wheels
    for (eCarTire currID: {eCarTire_Rear, eCarTire_Front})
    {
        GetTireCorners(currID, corners);

        for (int i = 0; i < 4; ++i)
        {
            points[i].x = corners[i].x;
            points[i].z = corners[i].y;
            points[i].y = drawHeight;
        }
        for (int i = 0; i < 4; ++i)
        {
            debugRender.DrawLine(points[i], points[(i + 1) % 4], Color32_Yellow, false);
        }

        glm::vec2 forwardVelocity = GetTireForwardVelocity(currID);
        glm::vec2 lateralVelocity = GetTireLateralVelocity(currID);
        glm::vec2 wheelPosition = GetTirePosition(currID);

        debugRender.DrawLine(
            glm::vec3 {wheelPosition.x, drawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + forwardVelocity.x, drawHeight, wheelPosition.y + forwardVelocity.y}, Color32_Green, false);
        debugRender.DrawLine(
            glm::vec3 {wheelPosition.x, drawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + lateralVelocity.x, drawHeight, wheelPosition.y + lateralVelocity.y}, Color32_Red, false);

        glm::vec2 signDirection = GetTireForward(currID);
        debugRender.DrawLine(
            glm::vec3 {wheelPosition.x, drawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + signDirection.x, drawHeight, wheelPosition.y + signDirection.y}, Color32_Yellow, false);
    }
}

SpriteDeltaBits Vehicle::GetSpriteDeltas() const
{
    SpriteDeltaBits deltaBits = mDamageDeltaBits;

    // add doors
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        if (!mDoorsAnims[idoor].IsNull())
        {
            unsigned int deltaBit = mDoorsAnims[idoor].GetSpriteIndex();
            deltaBits |= deltaBit;
        }
    }

    // add emergency lights
    if (mEmergLightsAnim.IsActive())
    {
        unsigned int deltaBit = mEmergLightsAnim.GetSpriteIndex();
        deltaBits |= deltaBit;
    }

    if (mDrivingDeltaAnim.IsActive())
    {
        unsigned int deltaBit = mDrivingDeltaAnim.GetSpriteIndex();
        deltaBits |= deltaBit;
    }

    return deltaBits;
}

void Vehicle::SetupDeltaAnimations()
{
    SpriteInfo& spriteInfo = gGameMap.mStyleData.mSprites[mSpriteIndex];
    SpriteDeltaBits deltaBits = spriteInfo.GetDeltaBits();

    mEmergLightsAnim.Clear();
    mDrivingDeltaAnim.Clear();
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        mDoorsAnims[idoor].Clear();
    }

    if (deltaBits == 0)
        return;

    SpriteDeltaBits maskBits = BIT(CAR_LIGHTING_SPRITE_DELTA_0) | BIT(CAR_LIGHTING_SPRITE_DELTA_1);
    if ((deltaBits & maskBits) == maskBits)
    {
        mEmergLightsAnim.mAnimDesc.mFrameRate = CAR_DELTA_ANIMS_SPEED;
        mEmergLightsAnim.mAnimDesc.SetFrames(
        {
            BIT(CAR_LIGHTING_SPRITE_DELTA_0), BIT(CAR_LIGHTING_SPRITE_DELTA_0), BIT(CAR_LIGHTING_SPRITE_DELTA_0),
            BIT(CAR_LIGHTING_SPRITE_DELTA_1), BIT(CAR_LIGHTING_SPRITE_DELTA_1), BIT(CAR_LIGHTING_SPRITE_DELTA_1),
        });
    }

    if (mCarInfo->mExtraDrivingAnim)
    {
        debug_assert(spriteInfo.mDeltaCount > CAR_DRIVE_SPRITE_DELTA);
        mDrivingDeltaAnim.mAnimDesc.mFrameRate = CAR_DELTA_DRIVING_ANIM_SPEED;
        mDrivingDeltaAnim.mAnimDesc.SetFrames(
        {
            0, BIT(CAR_DRIVE_SPRITE_DELTA),
            0, BIT(CAR_DRIVE_SPRITE_DELTA),
        });
    }

    // doors
    if (mCarInfo->mDoorsCount >= 1)
    {
        mDoorsAnims[0].mAnimDesc.mFrameRate = CAR_DELTA_ANIMS_SPEED;
        mDoorsAnims[0].mAnimDesc.SetFrames(
        {
            0, 
            BIT(CAR_DOOR1_SPRITE_DELTA_0), BIT(CAR_DOOR1_SPRITE_DELTA_1),
            BIT(CAR_DOOR1_SPRITE_DELTA_2), BIT(CAR_DOOR1_SPRITE_DELTA_3)
        });
        mDoorsAnims[0].SetFrameAction(0, eSpriteAnimAction_CarDoors);
        mDoorsAnims[0].SetListener(this);
    }

    if (mCarInfo->mDoorsCount >= 2)
    {
        mDoorsAnims[1].mAnimDesc.mFrameRate = CAR_DELTA_ANIMS_SPEED;
        mDoorsAnims[1].mAnimDesc.SetFrames(
        {
            0, 
            BIT(CAR_DOOR2_SPRITE_DELTA_0), BIT(CAR_DOOR2_SPRITE_DELTA_1),
            BIT(CAR_DOOR2_SPRITE_DELTA_2), BIT(CAR_DOOR2_SPRITE_DELTA_3)
        });
        mDoorsAnims[0].SetFrameAction(0, eSpriteAnimAction_CarDoors);
        mDoorsAnims[1].SetListener(this);
    }
}

void Vehicle::UpdateDeltaAnimations()
{  
    float deltaTime = gTimeManager.mGameFrameDelta;
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        if (mDoorsAnims[idoor].IsActive())
        {
            mDoorsAnims[idoor].UpdateFrame(deltaTime);
        }
    }

    if (mEmergLightsAnim.IsActive())
    {
        mEmergLightsAnim.UpdateFrame(deltaTime);
    }

    if (mCarInfo->mExtraDrivingAnim)
    {
        bool shouldEnable = fabs(GetCurrentSpeed()) > 0.5f; // todo: magic numbers
        if (mDrivingDeltaAnim.IsActive())
        {
            if (!shouldEnable)
            {
                mDrivingDeltaAnim.StopAnimation();
            }
            else
            {
                mDrivingDeltaAnim.UpdateFrame(deltaTime);
            }
        }
        else if (shouldEnable)
        {
            mDrivingDeltaAnim.PlayAnimation(eSpriteAnimLoop_FromStart);
        }
    }

    SpriteDeltaBits currDeltaBits = GetSpriteDeltas();
    if (mPrevDeltaBits != currDeltaBits)
    {
        mPrevDeltaBits = currDeltaBits;
        SetSprite(mSpriteIndex, currDeltaBits);
    }
}

void Vehicle::OpenDoor(int doorIndex)
{
    if (HasDoorAnimation(doorIndex))
    {
        if (IsDoorOpening(doorIndex) || IsDoorOpened(doorIndex))
            return;

        mDoorsAnims[doorIndex].PlayAnimation(eSpriteAnimLoop_None);
    }
}

void Vehicle::CloseDoor(int doorIndex)
{
    if (HasDoorAnimation(doorIndex))
    {
        if (IsDoorClosing(doorIndex) || IsDoorClosed(doorIndex))
            return;

        mDoorsAnims[doorIndex].RewindToEnd();
        mDoorsAnims[doorIndex].PlayAnimation(eSpriteAnimLoop_None, eSpriteAnimMode_Reverse);
    }
}

bool Vehicle::HasDoorAnimation(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return !mDoorsAnims[doorIndex].IsNull();
}

bool Vehicle::IsDoorOpened(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && !mDoorsAnims[doorIndex].IsActive() && 
        mDoorsAnims[doorIndex].IsLastFrame();
}

bool Vehicle::IsDoorClosed(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && !mDoorsAnims[doorIndex].IsActive() && 
        mDoorsAnims[doorIndex].IsFirstFrame();
}

bool Vehicle::IsDoorOpening(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && mDoorsAnims[doorIndex].IsActive() && 
        mDoorsAnims[doorIndex].IsRunsForwards();
}

bool Vehicle::IsDoorClosing(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && mDoorsAnims[doorIndex].IsActive() && 
        mDoorsAnims[doorIndex].IsRunsInReverse();
}

bool Vehicle::HasEmergencyLightsAnimation() const
{
    return !mEmergLightsAnim.IsNull();
}

bool Vehicle::IsEmergencyLightsEnabled() const
{
    return HasEmergencyLightsAnimation() && mEmergLightsAnim.IsActive();
}

void Vehicle::EnableEmergencyLights(bool isEnabled)
{
    if (HasEmergencyLightsAnimation())
    {
        if (isEnabled == mEmergLightsAnim.IsActive())
            return;

        if (isEnabled)
        {
            mEmergLightsAnim.PlayAnimation(eSpriteAnimLoop_FromStart);
        }
        else
        {
            mEmergLightsAnim.StopAnimation();
            mEmergLightsAnim.RewindToStart();
        }
    }
}

int Vehicle::GetDoorIndexForSeat(eCarSeat carSeat) const
{
    switch (carSeat)
    {
        case eCarSeat_Driver: return 0;
        case eCarSeat_Passenger: return 1;
        case eCarSeat_PassengerExtra: return 2;
        default: break;
    }
    debug_assert(false);
    return 0;
}

bool Vehicle::GetDoorPosLocal(int doorIndex, glm::vec2& out) const
{
    if (doorIndex > -1 && doorIndex < mCarInfo->mDoorsCount)
    {
        out.x = Convert::PixelsToMeters(mCarInfo->mDoors[doorIndex].mRpy);
        out.y = -Convert::PixelsToMeters(mCarInfo->mDoors[doorIndex].mRpx);
        return true;
    }

    debug_assert(false);
    return false;
}

bool Vehicle::GetDoorPos(int doorIndex, glm::vec2& out) const
{
    if (GetDoorPosLocal(doorIndex, out))
    {
        out = mPhysicsBody->GetWorldPoint(out);
        return true;
    }

    debug_assert(false);
    return false;
}

bool Vehicle::GetSeatPosLocal(eCarSeat carSeat, glm::vec2& out) const
{
    int doorIndex = GetDoorIndexForSeat(carSeat);

    glm::vec2 doorLocalPos;
    if (GetDoorPosLocal(doorIndex, doorLocalPos))
    {
        bool isBike = (mCarInfo->mClassID == eVehicleClass_Motorcycle);
        if (isBike)
        {
            out.y = 0.0f; // dead center
        }
        else
        {
            out.y = -mCarInfo->mDimensions.x * 0.25f * -glm::sign(doorLocalPos.y);
        }
        out.x = doorLocalPos.x;
        return true;
    }
    return false;
}

bool Vehicle::GetSeatPos(eCarSeat carSeat, glm::vec2& out) const
{
    if (GetSeatPosLocal(carSeat, out))
    {
        out = mPhysicsBody->GetWorldPoint(out);
        return true;
    }

    debug_assert(false);
    return false;
}

bool Vehicle::IsSeatPresent(eCarSeat carSeat) const
{
    int doorIndex = GetDoorIndexForSeat(carSeat);
    return (doorIndex < mCarInfo->mDoorsCount && doorIndex > -1);
}

void Vehicle::RegisterPassenger(Pedestrian* pedestrian, eCarSeat carSeat)
{
    if (pedestrian == nullptr || carSeat == eCarSeat_Any)
    {
        debug_assert(false);
        return;
    }

    if (pedestrian->IsAttachedToObject(this)) // already attached
    {
        debug_assert(false);
        return;
    }

    AttachObject(pedestrian);
    mPassengers.push_back(pedestrian);
}

void Vehicle::UnregisterPassenger(Pedestrian* pedestrian)
{
    if (pedestrian->IsAttachedToObject(this))
    {
        DetachObject(pedestrian);
    }

    cxx::erase_elements(mPassengers, pedestrian);
}

Pedestrian* Vehicle::GetCarDriver() const
{
    for (Pedestrian* currPassenger: mPassengers)
    {
        if (currPassenger->mCurrentSeat == eCarSeat_Driver)
            return currPassenger;
    }
    return nullptr;
}

Pedestrian* Vehicle::GetFirstPassenger(eCarSeat carSeat) const
{
    for (Pedestrian* currPassenger: mPassengers)
    {
        if (currPassenger->mCurrentSeat == carSeat)
            return currPassenger;
    }
    return nullptr;
}

void Vehicle::Explode()
{
    glm::vec3 explosionPos = mPhysicsBody->GetPosition();
    explosionPos.y = mDrawSprite.mHeight;

    mSpriteIndex = gGameMap.mStyleData.GetWreckedVehicleSpriteIndex(mCarInfo->mClassID);
    SetRemap(NO_REMAP);
    SetSprite(mSpriteIndex, GetSpriteDeltas());

    Explosion* explosion = gGameObjectsManager.CreateExplosion(this, nullptr, eExplosionType_CarDetonate, explosionPos);
    debug_assert(explosion);

    SetBurnEffectActive(true);

    // kill passengers inside
    for (Pedestrian* currentPed: mPassengers)
    {
        currentPed->DieFromDamage(eDamageCause_Explosion);
    }

    mExplosionWaitTime = 0.0f;
}

bool Vehicle::HasHardTop() const
{
    return !mCarInfo->mConvertible && (mCarInfo->mClassID != eVehicleClass_Motorcycle);
}

bool Vehicle::CanResistElectricity() const
{
    if ((mCarInfo->mClassID == eVehicleClass_Train) ||
        (mCarInfo->mClassID == eVehicleClass_Tram) ||
        (mCarInfo->mClassID == eVehicleClass_Tank))
    {
        return true;
    }
    return false;
}

bool Vehicle::IsWrecked() const
{
    return mCarWrecked;
}

bool Vehicle::ReceiveDamage(const DamageInfo& damageInfo)
{
    if (IsWrecked())
        return false;

    if (damageInfo.mDamageCause == eDamageCause_Electricity)
    {
        if (CanResistElectricity())
            return false;

        mCurrentDamage += 1; // todo: magic numbers
        return true;
    }

    if (damageInfo.mDamageCause == eDamageCause_Gravity)
    {
        float damageHeight = Convert::MapUnitsToMeters(1.3f); // todo: move to config
        if (damageInfo.mFallHeight >= damageHeight)
        {
            mDamageDeltaBits = CAR_DAMAGE_SPRITE_DELTA_MASK; // set all damages
            mCurrentDamage += 5; // todo: magic numbers
        }
        return true;
    }

    if (damageInfo.mDamageCause == eDamageCause_Explosion)
    {
        if (IsCriticalDamageState())
            return false; // ignore explosions

        if (damageInfo.mHitPoints == 1) // hack to simulate delayed explosion
        {
            mExplosionWaitTime = gGameParams.mCarExplosionChainDelayTime;
        }
        mCurrentDamage = 100; // force max damage
        return true;
    }

    if (damageInfo.mDamageCause == eDamageCause_Drowning)
    {
        SetWrecked();
        // kill passengers inside
        for (Pedestrian* currentPed: mPassengers)
        {
            currentPed->DieFromDamage(eDamageCause_Drowning);
        }
        return true;
    }

    if ((damageInfo.mDamageCause == eDamageCause_Bullet) ||
        (damageInfo.mDamageCause == eDamageCause_Burning))
    {
        mCurrentDamage += damageInfo.mHitPoints;
        return true;
    }

    if ((damageInfo.mDamageCause == eDamageCause_Collision) || (damageInfo.mDamageCause == eDamageCause_MapCollision))
    {
        if (damageInfo.mDamageCause == eDamageCause_Collision)
        {
            if (damageInfo.mSourceObject == nullptr || !damageInfo.mSourceObject->IsVehicleClass())
                return false;
        }
        if (damageInfo.mContactImpulse < 100.0f) // todo: magic numbers
            return false;

        glm::vec2 localPoint = mPhysicsBody->GetLocalPoint(damageInfo.mContactPoint.mPosition);

        // clockwise from top left corner
        glm::vec2 chassisCorners[4];
        GetChassisCornersLocal(chassisCorners);

        float miny = chassisCorners[1].y;
        float maxy = chassisCorners[2].y;
        float minx = chassisCorners[0].x;
        float maxx = chassisCorners[1].x;

        /*
                                Left
                         _______________________
                        |  |        |        |  |
                        |  |        |        |  |
                Rear    | --------- O --------- |    Front
                        |  |        |        |  |
                        |__|________|________|__|

                                Right
        */

        float fr_bias = (maxx / 3.0f); // front-rear offset from corners

        if (maxx - fabs(localPoint.x) < fr_bias)
        {
            if (localPoint.x > 0.0f)
            {
                // front left
                if (localPoint.y < 0.0f)
                {
                    mDamageDeltaBits |= BIT(CAR_DAMAGE_SPRITE_DELTA_FL);
                }
                // front right
                else
                {
                    mDamageDeltaBits |= BIT(CAR_DAMAGE_SPRITE_DELTA_FR);
                }
            }
            else
            {
                // rear left
                if (localPoint.y < 0.0f)
                {
                    mDamageDeltaBits |= BIT(CAR_DAMAGE_SPRITE_DELTA_BL);
                }
                // rear right
                else
                {
                    mDamageDeltaBits |= BIT(CAR_DAMAGE_SPRITE_DELTA_BR);
                }
            }
        }
        else
        {
            // middle left
            if (localPoint.y < 0.0f)
            {
                mDamageDeltaBits |= BIT(CAR_DAMAGE_SPRITE_DELTA_ML);
            }
            // middle right
            else
            {
                mDamageDeltaBits |= BIT(CAR_DAMAGE_SPRITE_DELTA_MR);
            }
        }

        ++mCurrentDamage;
        return true;
    }

    return false;
}

bool Vehicle::ShouldCollide(GameObject* otherObject) const
{
    if (otherObject->IsVehicleClass() || otherObject->IsObstacleClass())
        return true;

    return false;
}

void Vehicle::HandleCollision(const Collision& collision)
{
    DamageInfo damageInfo;
    damageInfo.SetDamageFromCollision(collision);
    ReceiveDamage(damageInfo);
}

void Vehicle::HandleCollisionWithMap(const MapCollision& collision)
{
    DamageInfo damageInfo;
    damageInfo.SetDamageFromCollision(collision);
    ReceiveDamage(damageInfo);
}

void Vehicle::HandleFallsOnWater(float fallDistance)
{
    mPhysicsBody->ClearForces();

    // boats aren't receive damage from water
    if (mCarInfo->mClassID == eVehicleClass_Boat)
        return;

    if (!IsWrecked())
    {
        DamageInfo damageInfo;
        damageInfo.mDamageCause = eDamageCause_Drowning;
        ReceiveDamage(damageInfo);
    }
}

void Vehicle::HandleFallsOnGround(float fallDistance)
{
    DamageInfo damageInfo;
    damageInfo.SetDamageFromFall(fallDistance);
    ReceiveDamage(damageInfo);
}

bool Vehicle::IsBurn() const
{
    return mFireEffect != nullptr;
}

void Vehicle::SetBurnEffectActive(bool activate)
{
    if (activate == IsBurn())
        return;

    if (activate)
    {
        debug_assert(mFireEffect == nullptr);
        GameObjectInfo& objectInfo = gGameMap.mStyleData.mObjects[GameObjectType_Fire1];
        mFireEffect = gGameObjectsManager.CreateDecoration(mTransform.mPosition, mTransform.mOrientation, &objectInfo);
        debug_assert(mFireEffect);
        if (mFireEffect)
        {
            mFireEffect->SetLifeDuration(0);
            mFireEffect->SetDrawOrder(eSpriteDrawOrder_CarRoof);
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

void Vehicle::UpdateBurnEffect()
{
    if (!IsBurn())
        return;

    if (gTimeManager.mGameTime > (mBurnStartTime + gGameParams.mCarBurnDuration))
    {
        SetBurnEffectActive(false);
        return;
    }
}

void Vehicle::SetWrecked()
{
    mCarWrecked = true;
}

bool Vehicle::HasPassengers() const
{
    return !mPassengers.empty();
}

void Vehicle::Repair()
{
    if (IsWrecked())
        return;

    mCurrentDamage = 0;
    mDamageDeltaBits = 0;
    mExplosionWaitTime = 0.0f;
}

int Vehicle::GetCurrentDamage() const
{
    return mCurrentDamage;
}

bool Vehicle::IsInWater() const
{
    return mPhysicsBody->mWaterContact;
}

void Vehicle::UpdateDamageFromRailways()
{
    if (IsWrecked() || CanResistElectricity())
        return;

    if (mPhysicsBody->mFalling)
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

bool Vehicle::HasDoorsOpened() const
{
    for (int icurrDoor = 0; icurrDoor < MAX_CAR_DOORS; ++icurrDoor)
    {
        if (HasDoorAnimation(icurrDoor) && !IsDoorClosed(icurrDoor))
            return true;
    }
    return false;
}

bool Vehicle::OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID)
{
    if (actionID == eSpriteAnimAction_CarDoors)
    {
        bool openDoors = animation->IsRunsForwards();
        StartGameObjectSound(eCarSfxChannelIndex_Doors, eSfxSampleType_Level, openDoors ? SfxLevel_CarDoorOpen : SfxLevel_CarDoorClose, SfxFlags_RandomPitch);
    }
    return true;
}

bool Vehicle::IsCriticalDamageState() const
{
    return mCurrentDamage >= 100;
}

void Vehicle::UpdateEngineSound()
{
    if (mSfxEmitter == nullptr)
    {
        InitSounds();
        if (mSfxEmitter == nullptr)
            return;
    }

    if (IsWrecked())
    {
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Engine);
        return;
    }

    if (!HasPassengers())
    {
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Engine);
        return;
    }

    SfxSampleIndex sfxIndex = SfxLevel_FirstCarEngineSound + mCarInfo->mEngine;

    if (!mSfxEmitter->IsPlaying(eCarSfxChannelIndex_Engine))
    {
        if (!StartGameObjectSound(eCarSfxChannelIndex_Engine, eSfxSampleType_Level, sfxIndex, SfxFlags_Loop))
            return;

        mSfxEmitter->SetGain(eCarSfxChannelIndex_Engine, 0.35f);
    }
    // todo: this is temporary solution!

    float speed = fabs(GetCurrentSpeed());
    float maxSpeed = 3.0f;
    float pitchValue = 0.8f + (speed / maxSpeed);
    pitchValue = std::min(pitchValue, 4.0f);
    mSfxEmitter->SetPitch(eCarSfxChannelIndex_Engine, pitchValue);
}

float Vehicle::GetCurrentSpeed() const
{
    if (mPhysicsBody)
    {
        glm::vec2 currentForwardNormal = mPhysicsBody->GetWorldVector(LocalForwardVector);
        glm::vec2 forwardVelocity = currentForwardNormal * glm::dot(currentForwardNormal, mPhysicsBody->GetLinearVelocity());
        return glm::dot(forwardVelocity, currentForwardNormal);
    }
    return 0.0f;
}

void Vehicle::GetChassisCorners(glm::vec2 corners[4]) const
{
    GetChassisCornersLocal(corners);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        corners[icorner] = mPhysicsBody->GetWorldPoint(corners[icorner]);
    }
}

void Vehicle::GetChassisCornersLocal(glm::vec2 corners[4]) const
{
    Collider* chassis = mPhysicsBody->GetColliderWithIndex(0);
    debug_assert(chassis);

    const CollisionShape& shapeData = chassis->mShapeData;
    debug_assert(shapeData.mType == eCollisionShape_Box);

    corners[0] = {shapeData.mBox.mCenter.x - shapeData.mBox.mHalfExtents.x, shapeData.mBox.mCenter.z - shapeData.mBox.mHalfExtents.z};
    corners[1] = {shapeData.mBox.mCenter.x + shapeData.mBox.mHalfExtents.x, shapeData.mBox.mCenter.z - shapeData.mBox.mHalfExtents.z};
    corners[2] = {shapeData.mBox.mCenter.x + shapeData.mBox.mHalfExtents.x, shapeData.mBox.mCenter.z + shapeData.mBox.mHalfExtents.z};
    corners[3] = {shapeData.mBox.mCenter.x - shapeData.mBox.mHalfExtents.x, shapeData.mBox.mCenter.z + shapeData.mBox.mHalfExtents.z};
}

void Vehicle::GetTireCorners(eCarTire tireID, glm::vec2 corners[4]) const
{
    debug_assert(tireID < eCarTire_COUNT);

    const float wheel_size_w = Convert::PixelsToMeters(CAR_WHEEL_SIZE_W_PX) * 0.5f;
    const float wheel_size_h = Convert::PixelsToMeters(CAR_WHEEL_SIZE_H_PX) * 0.5f;
    static const glm::vec2 points[4] =
    {
        glm::vec2(-wheel_size_h, -wheel_size_w),
        glm::vec2( wheel_size_h, -wheel_size_w),
        glm::vec2( wheel_size_h,  wheel_size_w),
        glm::vec2(-wheel_size_h,  wheel_size_w),
    };
    float positionOffset = 0.0f;
    if (tireID == eCarTire_Rear)
    {
        positionOffset = mRearTireOffset;
    }
    else
    {
        positionOffset = mFrontTireOffset;
    }

    for (int icorner = 0; icorner < 4; ++icorner)
    {
        glm::vec2 currPoint = points[icorner];
        if ((tireID == eCarTire_Front) && mSteeringAngleRadians)
        {
            currPoint = glm::rotate(points[icorner], mSteeringAngleRadians);
        }
        glm::vec2 point = mPhysicsBody->GetWorldPoint(currPoint + glm::vec2(positionOffset, 0.0f));
        corners[icorner] = point;
    }
}

void Vehicle::UpdateSteer(const DriveCtlState& currCtlState)
{
    const float LockAngleRadians = glm::radians(30.0f);
    const float TurnSpeedPerSec = glm::radians(270.0f * 1.0f);

    float turnPerTimeStep = (TurnSpeedPerSec * gTimeManager.mGameFrameDelta);
    float desiredAngle = (LockAngleRadians * currCtlState.mSteerDirection);
    float angleToTurn = glm::clamp((desiredAngle - mSteeringAngleRadians), -turnPerTimeStep, turnPerTimeStep);
    mSteeringAngleRadians = glm::clamp(mSteeringAngleRadians + angleToTurn, -LockAngleRadians, LockAngleRadians);
}

void Vehicle::UpdateFriction(const DriveCtlState& currCtlState)
{
    float linearSpeed = 0.0f;

    glm::vec2 linearVelocityVector = mPhysicsBody->GetLinearVelocity();
    if (glm::length2(linearVelocityVector) > 0.0f)
    {
        linearSpeed = glm::length(linearVelocityVector);
    }

    // kill lateral velocity front tire
    {
        glm::vec2 impulse = mPhysicsBody->GetMass() * 0.20f * -GetTireLateralVelocity(eCarTire_Front);
        mPhysicsBody->ApplyLinearImpulse(impulse, GetTirePosition(eCarTire_Front));
    }

    // kill lateral velocity rear tire
    {
        glm::vec2 impulse = mPhysicsBody->GetMass() * 0.20f * -GetTireLateralVelocity(eCarTire_Rear);
        mPhysicsBody->ApplyLinearImpulse(impulse, GetTirePosition(eCarTire_Rear));
    }

    // rolling resistance
    if (linearSpeed > 0.0f)
    {
        float rrCoef = 50.0f;
        mPhysicsBody->ApplyLinearImpulse(rrCoef * -linearVelocityVector, GetTirePosition(eCarTire_Front));
        mPhysicsBody->ApplyLinearImpulse(rrCoef * -linearVelocityVector, GetTirePosition(eCarTire_Rear));
    }

    // apply drag force
    if (linearSpeed > 0.0f)
    {
        float dragForceCoef = 102.0f;
        glm::vec2 dragForce = -dragForceCoef * linearSpeed * linearVelocityVector;

        mPhysicsBody->AddForce(dragForce);
    }
}

void Vehicle::UpdateDrive(const DriveCtlState& currCtlState)
{
    if (currCtlState.mDriveDirection == 0.0f)
        return;

    float driveForce = 100750.0f; // todo: magic numbers
    float brakeForce = driveForce * mCarInfo->mHandbrakeFriction;
    float reverseForce = driveForce * 0.75f;

    float currentSpeed = GetCurrentSpeed();
    float engineForce = 0.0f;

    if (currCtlState.mDriveDirection > 0.0f)
    {
        engineForce = driveForce;
    }
    else
    {
        if (currentSpeed > 0.0f)
        {
            engineForce = brakeForce;
        }
        else
        {
            engineForce = reverseForce;
        }
    }

    glm::vec2 F = engineForce * currCtlState.mDriveDirection * GetTireForward(eCarTire_Rear);
    mPhysicsBody->AddForce(F, GetTirePosition(eCarTire_Rear));
}

glm::vec2 Vehicle::GetTireLateralVelocity(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    glm::vec2 normal_vector = GetTireLateral(tireID);
    glm::vec2 local_position = GetTireLocalPos(tireID);

    return normal_vector * glm::dot(normal_vector, mPhysicsBody->GetLinearVelocityFromLocalPoint(local_position));
}

glm::vec2 Vehicle::GetTireForwardVelocity(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    glm::vec2 normal_vector = GetTireForward(tireID);
    glm::vec2 local_position = GetTireLocalPos(tireID);

    return normal_vector * glm::dot(normal_vector, mPhysicsBody->GetLinearVelocityFromLocalPoint(local_position));
}

glm::vec2 Vehicle::GetTirePosition(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    glm::vec2 local_position = GetTireLocalPos(tireID);
    return mPhysicsBody->GetWorldPoint(local_position);
}

glm::vec2 Vehicle::GetTireForward(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    glm::vec2 local_vector = GetTireLocalForward(tireID);
    return mPhysicsBody->GetWorldVector(local_vector);
}

glm::vec2 Vehicle::GetTireLateral(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    glm::vec2 local_vector = GetTireLocalLateral(tireID);
    return mPhysicsBody->GetWorldVector(local_vector);
}

glm::vec2 Vehicle::GetTireLocalPos(eCarTire tireID) const
{
    if (tireID == eCarTire_Rear)
    {
        return (LocalForwardVector * mRearTireOffset);
    }
    if (tireID == eCarTire_Front)
    {
        return (LocalForwardVector * mFrontTireOffset);
    }
    debug_assert(false);
    return {};
}

glm::vec2 Vehicle::GetTireLocalForward(eCarTire tireID) const
{
    if (tireID == eCarTire_Rear)
    {
        return LocalForwardVector;
    }
    if (tireID == eCarTire_Front)
    {
        return glm::rotate(LocalForwardVector, mSteeringAngleRadians);
    }
    debug_assert(false);
    return {};
}

glm::vec2 Vehicle::GetTireLocalLateral(eCarTire tireID) const
{
    if (tireID == eCarTire_Rear)
    {
        return LocalLateralVector;
    }
    if (tireID == eCarTire_Front)
    {
        return glm::rotate(LocalLateralVector, mSteeringAngleRadians);
    }
    debug_assert(false);
    return {};
}

void Vehicle::SetRemap(int remapIndex)
{
    mRemapIndex = remapIndex;
    mRemapClut = (mRemapIndex == NO_REMAP) ? 0 : (mCarInfo->mRemapsBaseIndex + mRemapIndex);
}
