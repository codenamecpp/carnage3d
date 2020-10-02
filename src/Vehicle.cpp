#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "Pedestrian.h"
#include "TimeManager.h"
#include "GameObjectsManager.h"
#include "AudioManager.h"

Vehicle::Vehicle(GameObjectID id) : GameObject(eGameObjectClass_Car, id)
    , mPhysicsBody()
    , mCarWrecked()
    , mCarInfo()
    , mDamageDeltaBits()
    , mDrivingDeltaAnim()
    , mDrawHeight()
{
}

Vehicle::~Vehicle()
{
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
    
    if (mPhysicsBody)
    {
        gPhysics.DestroyPhysicsObject(mPhysicsBody);
    }
    gSpriteManager.FlushSpritesCache(mObjectID);
}

void Vehicle::Spawn(const glm::vec3& position, cxx::angle_t heading)
{
    mSpawnPosition = position;
    mSpawnHeading = heading;

    mStandingOnRailwaysTimer = 0.0f;

    debug_assert(mCarInfo);
    
    if (mPhysicsBody == nullptr)
    {
        mPhysicsBody = gPhysics.CreatePhysicsObject(this, position, heading);
        debug_assert(mPhysicsBody);
    }
    else
    {   
        mPhysicsBody->SetPosition(position, heading);
    }

    mCarWrecked = false;
    mCurrentDamage = 0;
    mDamageDeltaBits = 0;
    mSpriteIndex = mCarInfo->mSpriteIndex; // todo: handle bike fallen state 

    SetupDeltaAnimations();
    SetBurnEffectActive(false);
}

void Vehicle::UpdateFrame()
{
    UpdateBurnEffect();
    UpdateDamageFromRailways();

    if (IsWrecked())
        return;

    // check if car destroyed
    if (mCurrentDamage >= 100)
    {
        SetWrecked();
        Explode();
        return;
    }

    UpdateDeltaAnimations();
}

void Vehicle::PreDrawFrame()
{   
    // sync sprite transformation with physical body
    cxx::angle_t rotationAngle = mPhysicsBody->mSmoothRotation;
    glm::vec3 position = mPhysicsBody->mSmoothPosition;
    ComputeDrawHeight(position);

    int remapClut = mRemapIndex == NO_REMAP ? 0 : (mCarInfo->mRemapsBaseIndex + mRemapIndex);
    gSpriteManager.GetSpriteTexture(mObjectID, mSpriteIndex, remapClut, GetSpriteDeltas(), mDrawSprite);

    mDrawSprite.mPosition.x = position.x;
    mDrawSprite.mPosition.y = position.z;
    mDrawSprite.mRotateAngle = rotationAngle - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    mDrawSprite.mHeight = mDrawHeight;
    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Car;

    // update fire effect draw pos
    // todo: refactore
    if (mFireEffect)
    {
        mFireEffect->SetTransform(position, rotationAngle);
    }
}

void Vehicle::DebugDraw(DebugRenderer& debugRender)
{
    glm::vec3 position = mPhysicsBody->GetPosition();

    glm::vec2 corners[4];
    mPhysicsBody->GetChassisCorners(corners);

    glm::vec3 points[4];
    for (int i = 0; i < 4; ++i)
    {
        points[i].x = corners[i].x;
        points[i].z = corners[i].y;
        points[i].y = mDrawHeight;
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
        glm::vec3 {position.x, mDrawHeight, position.z},
        glm::vec3 {position.x + bodyLinearVelocity.x, mDrawHeight, position.z + bodyLinearVelocity.y}, Color32_Cyan, false);

    // draw wheels
    for (eCarTire currID: {eCarTire_Rear, eCarTire_Front})
    {
        mPhysicsBody->GetTireCorners(currID, corners);

        for (int i = 0; i < 4; ++i)
        {
            points[i].x = corners[i].x;
            points[i].z = corners[i].y;
            points[i].y = mDrawHeight;
        }
        for (int i = 0; i < 4; ++i)
        {
            debugRender.DrawLine(points[i], points[(i + 1) % 4], Color32_Yellow, false);
        }
        glm::vec2 forwardVelocity = mPhysicsBody->GetTireForwardVelocity(currID);
        glm::vec2 lateralVelocity = mPhysicsBody->GetTireLateralVelocity(currID);
        glm::vec2 wheelPosition = mPhysicsBody->GetTirePosition(currID);

        debugRender.DrawLine(
            glm::vec3 {wheelPosition.x, mDrawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + forwardVelocity.x, mDrawHeight, wheelPosition.y + forwardVelocity.y}, Color32_Green, false);
        debugRender.DrawLine(
            glm::vec3 {wheelPosition.x, mDrawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + lateralVelocity.x, mDrawHeight, wheelPosition.y + lateralVelocity.y}, Color32_Red, false);

        glm::vec2 signDirection = mPhysicsBody->GetTireForward(currID);
        debugRender.DrawLine(
            glm::vec3 {wheelPosition.x, mDrawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + signDirection.x, mDrawHeight, wheelPosition.y + signDirection.y}, Color32_Yellow, false);
    }
}

glm::vec3 Vehicle::GetPosition() const
{
    return mPhysicsBody->GetPosition();
}

glm::vec2 Vehicle::GetPosition2() const
{
    return mPhysicsBody->GetPosition2();
}

void Vehicle::ComputeDrawHeight(const glm::vec3& position)
{
    glm::vec2 corners[4];
    mPhysicsBody->GetChassisCorners(corners);

    float maxHeight = position.y;

    for (int icorner = 0; icorner < 4; ++icorner)
    {
        glm::vec3 cornerPosition { corners[icorner].x, position.y, corners[icorner].y };
        float cornerHeight = gGameMap.GetHeightAtPosition(cornerPosition);
        if (cornerHeight > maxHeight)
        {
            maxHeight = cornerHeight;
        }
    }

    mDrawHeight = maxHeight;
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
        bool shouldEnable = fabs(mPhysicsBody->GetCurrentSpeed()) > 0.5f; // todo: magic numbers
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

    pedestrian->SetAttachedToObject(this);
    mPassengers.push_back(pedestrian);
}

void Vehicle::UnregisterPassenger(Pedestrian* pedestrian)
{
    if (pedestrian->IsAttachedToObject(this))
    {
        pedestrian->SetDetached();
        cxx::erase_elements(mPassengers, pedestrian);
    }
    else
    {
        debug_assert(false);
    }
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
    explosionPos.y = mDrawHeight;

    mSpriteIndex = gGameMap.mStyleData.GetWreckedVehicleSpriteIndex(mCarInfo->mClassID);
    mRemapIndex = NO_REMAP;

    Explosion* explosion = gGameObjectsManager.CreateExplosion(explosionPos);
    debug_assert(explosion);
    if (explosion)
    {
        explosion->DisablePrimaryDamage();
    }

    SetBurnEffectActive(true);

    // kill passengers inside
    for (Pedestrian* currentPed: mPassengers)
    {
        currentPed->DieFromDamage(eDamageCause_Explosion);
    }
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
        mCurrentDamage += damageInfo.mHitPoints;
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

    if (damageInfo.mDamageCause == eDamageCause_Bullet ||
        damageInfo.mDamageCause == eDamageCause_Burning)
    {
        mCurrentDamage += damageInfo.mHitPoints;
        return true;
    }

    if (damageInfo.mDamageCause == eDamageCause_CarCrash)
    {
        if (damageInfo.mSourceObject == nullptr || !damageInfo.mSourceObject->IsVehicleClass())
            return false;

        glm::vec2 localPoint = mPhysicsBody->GetLocalPoint(glm::vec2(damageInfo.mContactPoint.x, damageInfo.mContactPoint.z));

        // clockwise from top left corner
        glm::vec2 chassisCorners[4];
        mPhysicsBody->GetLocalChassisCorners(chassisCorners);

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
        mFireEffect = gGameObjectsManager.CreateDecoration(
            mPhysicsBody->GetPosition(), 
            mPhysicsBody->GetRotationAngle(), &objectInfo);
        debug_assert(mFireEffect);
        if (mFireEffect)
        {
            mFireEffect->SetLifeDuration(0);
            mFireEffect->SetAttachedToObject(this);
        }
        mFireEffect->SetDrawOrder(eSpriteDrawOrder_Car);
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

void Vehicle::UpdateBurnEffect()
{
    if (!IsBurn())
        return;

    if (gTimeManager.mGameTime > (mBurnStartTime + gGameParams.mVehicleBurnDuration))
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

    glm::ivec3 logPosition = Convert::MetersToMapUnits(GetPosition());
    
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
        gAudioManager.PlaySfxLevel(openDoors ? SfxLevel_CarDoorOpen : SfxLevel_CarDoorClose, GetPosition(), false);
    }
    return true;
}
