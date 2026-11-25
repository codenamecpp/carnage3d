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
#include "GameObjectHelpers.h"
#include "BroadcastEventsManager.h"
#include "CarnageGame.h"

// original game physics constants
static const float CarMassScale = 60.0f; // style data mass to kilograms
static const float CarRearTyreCoef = 1.5f;
static const float CoachRearTyreCoef = 3.0f;
static const float CarHandBrakeLateralCoef = 0.6f;
static const float CarSteeringLockAngle = 0.873f; // radians
static const float CarSteeringCenterAngle = 0.139f; // radians
static const float CarSteeringRecenterSpeed = 3.24f; // radians per second, 0.139 per tick
static const float CarRoadAssistSpeed = 0.65f; // radians per second, 0.0278 per tick
static const float CarRampRiseSpeed = 5.83f; // meters per second, 4 pixels per tick
static const float CarRoadAssistAngle = glm::radians(11.25f);
static const float CarRoadAssistLockAngle = glm::pi<float>() / 1024.0f;
static const int CarEngineSampleRates[] = { 18000, 33000, 41000, 48000, 53000, 58000, 62000, 66000, 69500, 72000, 74000, 76500, 77000 };

// engine and skid sounds get louder with speed
static float GetCarSoundGain(int speed)
{
    if (speed < 1)
        return 0.4f;

    return std::min(speed * 1.5f + 40.0f, 100.0f) / 100.0f;
}

static int GetCarEngineSampleRate(const VehicleInfo* carInfo, SfxSampleIndex sfxIndex, int speed)
{
    // gear changes every 10 pixels per tick
    auto getGearsSampleRate = [](int speed)
    {
        if (speed < 0)
            return CarEngineSampleRates[std::min(-speed, 12)];

        return CarEngineSampleRates[std::min(speed / 10 + speed % 10 + (speed > 0 ? 1 : 0), 12)];
    };

    switch (carInfo->mSoundFunction)
    {
        case 0:
        {
            int sampleRate = getGearsSampleRate(speed);
            if (sfxIndex == SfxLevel_FirstCarEngineSound + 9)
                return sampleRate * 2;

            return sampleRate;
        }
        case 1: return getGearsSampleRate(speed) / 2;
        case 2: return 22050 + ((speed < 0) ? -speed * 400 : speed * 1400);
        case 3: return (22050 + ((speed < 0) ? -speed * 400 : speed * 1400)) * 3;
        case 4: return (std::abs(speed) + 2) * 2000;
        case 5:
        {
            if (speed < 0) return (12 - speed) * 500;
            if (speed < 8) return speed * 333 + 6000;
            if (speed < 12) return speed * 750;
            if (speed < 18) return speed * 500;
            return speed * 333;
        }
    }
    return 0;
}

struct SkidMarkArea
{
    int mBlockTyreSmokeCount = 0;
    int mBlockSkidMarksCount = 0;
    bool mIsBloodNear = false;
};

static SkidMarkArea ScanSkidMarkArea(const glm::vec3& position, const glm::ivec2& mapBlock)
{
    const float maxBloodDistance = Convert::PixelsToMeters(8.0f);

    SkidMarkArea area;
    for (GameObject* gameObject: gGameObjectsManager.mAllObjects)
    {
        bool isBlood = false;
        if (gameObject->IsDecorationClass())
        {
            Decoration* decoration = static_cast<Decoration*>(gameObject);
            int objectType = decoration->GetObjectType();
            isBlood = (objectType == GameObjectType_Blood) || (objectType == GameObjectType_FirstBlood) || (objectType == GameObjectType_Body);

            bool isSkidMark = decoration->IsSkidMark();
            bool isTyreSmoke = (objectType == GameObjectType_TyreSmoke);
            if ((isSkidMark || isTyreSmoke) && !decoration->IsMarkedForDeletion() &&
                (glm::ivec2(Convert::MetersToMapUnits(decoration->mTransform.GetPosition2())) == mapBlock))
            {
                if (isSkidMark)
                {
                    ++area.mBlockSkidMarksCount;
                }
                else
                {
                    ++area.mBlockTyreSmokeCount;
                }
            }
        }
        else if (gameObject->IsPedestrianClass())
        {
            isBlood = static_cast<Pedestrian*>(gameObject)->IsDead();
        }

        if (isBlood && !area.mIsBloodNear)
        {
            glm::vec3 distance = glm::abs(gameObject->mTransform.mPosition - position);
            area.mIsBloodNear = (distance.x < maxBloodDistance) && (distance.y < maxBloodDistance) && (distance.z < maxBloodDistance);
        }
    }
    return area;
}

static bool IsDirtSkidMarkLid(int lid)
{
    static const std::vector<int> CityLids[] =
    {
        { 10, 58, 78, 79, 83, 107, 108, 116 },
        { 10, 13, 59, 70, 78, 79, 83, 88, 93, 94, 98, 106, 107, 111, 128, 129, 130, 131, 132, 133, 141, 143, 162 },
        { 10, 17, 18, 29, 30, 31, 32, 33, 35, 52, 53, 78, 79, 107, 108, 121, 122, 123, 124, 125, 132 },
    };

    int cityIndex = gGameMap.mStyleFileNumber - 1;
    if ((cityIndex < 0) || (cityIndex >= (int) CountOf(CityLids)))
        return false;

    const std::vector<int>& lids = CityLids[cityIndex];
    return std::find(lids.begin(), lids.end(), lid) != lids.end();
}

static void SetCarSoundSampleRate(SfxEmitter* sfxEmitter, int ichannel, SfxSampleIndex sfxIndex, int sampleRate)
{
    SfxSample* sfxSample = gAudioManager.GetSound(eSfxSampleType_Level, sfxIndex);
    if (sfxSample && (sfxSample->GetSampleRate() > 0))
    {
        sfxEmitter->SetPitch(ichannel, (1.0f * sampleRate) / sfxSample->GetSampleRate());
    }
}

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

    // style data has center of mass and moment of inertia in pixels
    glm::vec2 centerOfMass = Convert::PixelsToMeters(glm::ivec2(mCarInfo->mCy, mCarInfo->mCx));
    float rotationalInertia = mCarInfo->mMoment * MAP_SPRITE_SCALE * MAP_SPRITE_SCALE;
    physicsBody->SetMassData(mCarInfo->mMass * CarMassScale, centerOfMass, rotationalInertia * CarMassScale);

    mSteeringAngleWorld = physicsBody->GetOrientation().to_radians();
    mSkidValue = 0.0f;
    mSkidMarkTime = 0.0f;
    mBloodSkidMarks[0] = mBloodSkidMarks[1] = 0;
    mCtlState = DriveCtlState();
    mStepStartVelocity = glm::vec2();
    mFallDamagePixels = 0.0f;
    mFallPrevHeight = 0.0f;
    mMapContact = false;
    mPrevMapContact = false;

    mCarWrecked = false;
    mCurrentDamage = 0;
    mDamageDeltaBits = 0;
    mPrevDeltaBits = 0;
    mSpriteIndex = mCarInfo->mSpriteIndex; // todo: handle bike fallen state 

    SetupDeltaAnimations();
    SetRemap(NO_REMAP);

    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Car;
    SetSprite(mSpriteIndex, GetSpriteDeltas());

    CreateTurret(cxx::angle_t());
}

void Vehicle::HandleDespawn()
{
    SetBurnEffectActive(false);  
    DestroyTurret();

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
    UpdateTurret();
    UpdateDamageFromRailways();
    UpdateEngineSound();
    UpdateHorn();
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
            const PedestrianCtlState& ctlState = carDriver->GetCtlState();
            currCtlState.mDriveDirection = ctlState.mAcceleration;
            bool isTurretControl = (mTurret != nullptr) && ctlState.mHorn;
            currCtlState.mSteerDirection = isTurretControl ? 0.0f : ctlState.mSteerDirection;
            currCtlState.mHandBrake = ctlState.mHandBrake;
        }
    }
    mCtlState = currCtlState;

    UpdateMapContactDamage();
    UpdateFallDamage();
    UpdateSteer(currCtlState);
    UpdateTyreForces(currCtlState);
    UpdateSkidMarks();

    mStepStartVelocity = mPhysicsBody->GetLinearVelocity();
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
    if (mTurret)
    {
        DestroyTurret();
        CreateTurret(mTurretAngle);
    }

    Explosion* explosion = gGameObjectsManager.CreateExplosion(this, nullptr, eExplosionType_CarDetonate, explosionPos);
    debug_assert(explosion);

    SetBurnEffectActive(true);

    // kill passengers inside
    DamageInfo damageInfo;
    damageInfo.SetExplosionDamage(nullptr);
    for (Pedestrian* currentPed: mPassengers)
    {
        currentPed->DieFromDamage(damageInfo);
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
        // landing doubles current damage, but not on slope
        if (!HasCornerOnSlope())
        {
            AddDamage(mCurrentDamage);
        }
        if ((mCurrentDamage > 50) && !IsCriticalDamageState())
        {
            mDamageDeltaBits = CAR_DAMAGE_SPRITE_DELTA_MASK; // set all damages
        }
        gAudioManager.StartSound(eSfxSampleType_Level, SfxLevel_CarLanding, SfxFlags_None, mTransform.mPosition);
        return true;
    }

    if ((damageInfo.mDamageCause == eDamageCause_Explosion) ||
        (damageInfo.mDamageCause == eDamageCause_ExplosionChain))
    {
        if (IsCriticalDamageState())
            return false; // ignore explosions

        if (damageInfo.mDamageCause == eDamageCause_ExplosionChain) // delayed explosion
        {
            mExplosionWaitTime = gGameParams.mCarExplosionChainDelayTime;
        }
        mCurrentDamage = GetMaxDamage(); // force max damage
        return true;
    }

    if (damageInfo.mDamageCause == eDamageCause_Water)
    {
        SetWrecked();
        // kill passengers inside
        for (Pedestrian* currentPed: mPassengers)
        {
            currentPed->DieFromDamage(damageInfo);
        }
        return true;
    }

    if ((damageInfo.mDamageCause == eDamageCause_Bullet) ||
        (damageInfo.mDamageCause == eDamageCause_Flame))
    {
        mCurrentDamage += damageInfo.mVehicleDamage;
        return true;
    }

    if (damageInfo.mDamageCause == eDamageCause_Collision)
    {
        Vehicle* otherCar = ToVehicle(damageInfo.mSourceObject);
        if (otherCar == nullptr)
            return false;

        // cars explode when they hit each other in air
        if (IsInAir() || otherCar->IsInAir())
        {
            AddDamage(GetMaxDamage());
            return true;
        }

        // ignore cars that only touch each other
        if (damageInfo.mImpactSpeed < gGameParams.mCarMinImpactSpeed)
            return false;

        if ((mCarInfo->mModelID != eVehicle_ModelCar) && ((std::max(GetSpeedPixels(), otherCar->GetSpeedPixels()) / 2) > 6))
        {
            SetDamageDeltaAtPoint(damageInfo.mContactPoint.mPosition);
        }

        int collisionDamage = ComputeCollisionDamage(otherCar);
        if (collisionDamage < 1)
            return false;

        AddDamage(collisionDamage);
        return true;
    }

    return false;
}

void Vehicle::SetDamageDeltaAtPoint(const glm::vec2& worldPoint)
{
    glm::vec2 localPoint = mPhysicsBody->GetLocalPoint(worldPoint);

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
    damageInfo.SetCollisionDamage(collision);
    ReceiveDamage(damageInfo);
}

void Vehicle::HandleCollisionWithMap(const MapCollision& collision)
{
    if (!mMapContact && !mPrevMapContact && !IsWrecked() && (GetSpeedPixels() > 1) && collision.HasContactPoints())
    {
        SetDamageDeltaAtPoint(collision.mContactPoints[0].mPosition);
    }
    mMapContact = true;

    // car cannot turn while it touches wall
    mPhysicsBody->SetAngularVelocity(cxx::angle_t());
}

void Vehicle::HandleFallingStarts()
{
    // faster car stays in air longer
    mPhysicsBody->mFallHoverTime = (GetSpeedPixels() / 4) / GTA_TICKS_PER_SECOND;

    // car jumps up from ramp
    if (HasCornerOnSlope())
    {
        mPhysicsBody->mPositionY += Convert::PixelsToMeters(16);
    }
    mFallPrevHeight = mPhysicsBody->mPositionY;
    mFallDamagePixels = 0.0f;
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
        damageInfo.mDamageCause = eDamageCause_Water;
        ReceiveDamage(damageInfo);
    }
}

void Vehicle::HandleFallsOnGround(float fallDistance)
{
    DamageInfo damageInfo;
    damageInfo.SetFallDamage(fallDistance);
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

static Decoration* CreateCarPart(int objectType, const Transform& transform, eSpriteDrawOrder drawOrder)
{
    GameObjectInfo objectInfo = gGameMap.mStyleData.mObjects[objectType];
    objectInfo.mClassID = eGameObjectClass_Decoration;

    Decoration* carPart = gGameObjectsManager.CreateDecoration(transform.mPosition, transform.mOrientation, &objectInfo);
    debug_assert(carPart);
    carPart->SetDrawOrder(drawOrder);
    return carPart;
}

static float GetCarPartLength(int objectType)
{
    const GameObjectInfo& objectInfo = gGameMap.mStyleData.mObjects[objectType];
    int spriteIndex = objectInfo.mAnimationData.mFrames[0].mSprite;
    return Convert::PixelsToMeters(gGameMap.mStyleData.mSprites[spriteIndex].mHeight);
}

void Vehicle::CreateTurret(cxx::angle_t turretAngle)
{
    const int FireHoseMountOffset = 30; // pixels behind the car center
    const int FireHosePivotOffset = 6; // pixels from the hose pivot to the hose sprite center

    debug_assert(mTurret == nullptr);

    int turretObjectType = GameObjectType_Null;
    int barrelObjectType = GameObjectType_Null;
    mTurretMountPosition = glm::vec3();
    mTurretPivotOffset = 0.0f;

    if (mCarInfo->mClassID == eVehicleClass_Tank)
    {
        turretObjectType = IsWrecked() ? GameObjectType_TankGunWrecked : GameObjectType_TankGun;
        barrelObjectType = IsWrecked() ? GameObjectType_TankGunBarrelWrecked : GameObjectType_TankGunBarrel;
    }
    else if ((mCarInfo->mModelID == eVehicle_FireTruck) && !IsWrecked())
    {
        turretObjectType = GameObjectType_FireHose;
        mTurretMountPosition.x = -Convert::PixelsToMeters(FireHoseMountOffset);
        mTurretPivotOffset = Convert::PixelsToMeters(FireHosePivotOffset);
    }

    if (turretObjectType == GameObjectType_Null)
        return;

    mTurret = CreateCarPart(turretObjectType, mTransform, eSpriteDrawOrder_CarRoof);
    AttachObject(mTurret);
    mTurretMuzzleOffset = GetCarPartLength(turretObjectType) * 0.5f;

    if (barrelObjectType != GameObjectType_Null)
    {
        mTurretBarrel = CreateCarPart(barrelObjectType, mTransform, eSpriteDrawOrder_ConvetibleCarPassenger);
        mTurret->AttachObject(mTurretBarrel);

        float barrelOffset = GetCarPartLength(barrelObjectType) * 0.5f;
        mTurretBarrel->SetTransform(glm::vec3(barrelOffset, 0.0f, 0.0f), cxx::angle_t(), eTransformSpace_Local);
        mTurretMuzzleOffset = barrelOffset * 2.0f;
    }

    SetTurretAngle(turretAngle);
}

void Vehicle::DestroyTurret()
{
    if (mTurret == nullptr)
        return;

    SetWaterJetActive(false);

    if (mTurretBarrel)
    {
        mTurret->DetachObject(mTurretBarrel);
        mTurretBarrel->MarkForDeletion();
        mTurretBarrel = nullptr;
    }

    DetachObject(mTurret);
    mTurret->MarkForDeletion();
    mTurret = nullptr;
}

void Vehicle::SetTurretAngle(cxx::angle_t turretAngle)
{
    debug_assert(mTurret);

    mTurretAngle = turretAngle;

    float sinAngle, cosAngle;
    turretAngle.get_sin_cos(sinAngle, cosAngle);

    glm::vec3 turretPosition = mTurretMountPosition;
    turretPosition.x += cosAngle * mTurretPivotOffset;
    turretPosition.z += sinAngle * mTurretPivotOffset;
    mTurret->SetTransform(turretPosition, turretAngle, eTransformSpace_Local);
}

void Vehicle::UpdateTurret()
{
    Pedestrian* carDriver = IsWrecked() ? nullptr : GetCarDriver();
    if ((mTurret == nullptr) || (carDriver == nullptr))
    {
        SetWaterJetActive(false);
        return;
    }

    const PedestrianCtlState& ctlState = carDriver->GetCtlState();
    if (ctlState.mHorn && (ctlState.mSteerDirection != 0.0f))
    {
        float rotationDegrees = ctlState.mSteerDirection * gGameParams.mTurretRotationSpeed * gTimeManager.mGameFrameDelta;
        SetTurretAngle(mTurretAngle + cxx::angle_t::from_degrees(rotationDegrees));
    }

    if (mCarInfo->mClassID == eVehicleClass_Tank)
    {
        if (ctlState.mShoot)
        {
            FireTankGun(carDriver);
        }
    }
    else
    {
        UpdateWaterJet(ctlState.mShoot);
    }
}

glm::vec2 Vehicle::GetTurretMuzzlePosition() const
{
    debug_assert(mTurret);

    return mTurret->mTransform.GetPoint2(glm::vec2(mTurretMuzzleOffset, 0.0f), eTransformSpace_World);
}

void Vehicle::SetWaterJetActive(bool isActive)
{
    const int WaterJetSegmentsCount = 4;

    if (isActive == !mWaterJetParts.empty())
        return;

    if (isActive)
    {
        debug_assert(mTurret);

        for (int isegment = 0; isegment < WaterJetSegmentsCount; ++isegment)
        {
            Decoration* jetSegment = CreateCarPart(GameObjectType_WaterJet, mTurret->mTransform, eSpriteDrawOrder_Projectiles);
            mTurret->AttachObject(jetSegment);
            mWaterJetParts.push_back(jetSegment);
        }

        Decoration* jetEnd = CreateCarPart(GameObjectType_WaterJetEnd, mTurret->mTransform, eSpriteDrawOrder_Projectiles);
        mTurret->AttachObject(jetEnd);
        mWaterJetParts.push_back(jetEnd);
    }
    else
    {
        for (Decoration* currPart: mWaterJetParts)
        {
            mTurret->DetachObject(currPart);
            currPart->MarkForDeletion();
        }
        mWaterJetParts.clear();
    }
}

void Vehicle::SetWaterJetLength(float jetLength)
{
    debug_assert(!mWaterJetParts.empty());

    float segmentLength = GetCarPartLength(GameObjectType_WaterJet);
    float segmentsLength = jetLength - GetCarPartLength(GameObjectType_WaterJetEnd) * 0.5f;

    Decoration* jetEnd = mWaterJetParts.back();
    jetEnd->SetTransform(glm::vec3(mTurretMuzzleOffset + jetLength, 0.0f, 0.0f), cxx::angle_t(), eTransformSpace_Local);

    for (size_t isegment = 0; (isegment + 1) < mWaterJetParts.size(); ++isegment)
    {
        Decoration* jetSegment = mWaterJetParts[isegment];

        float segmentStart = segmentLength * isegment;
        if (segmentStart < segmentsLength)
        {
            jetSegment->mObjectFlags = (jetSegment->mObjectFlags & ~GameObjectFlags_Invisible);
        }
        else
        {
            jetSegment->mObjectFlags = (jetSegment->mObjectFlags | GameObjectFlags_Invisible);
        }

        // the last visible segment moves back to end at the splash
        segmentStart = glm::max(glm::min(segmentStart, segmentsLength - segmentLength), 0.0f);
        jetSegment->SetTransform(glm::vec3(mTurretMuzzleOffset + segmentStart + segmentLength * 0.5f, 0.0f, 0.0f), cxx::angle_t(), eTransformSpace_Local);
    }
}

void Vehicle::UpdateWaterJet(bool isActive)
{
    SetWaterJetActive(isActive);
    if (!isActive)
        return;

    int jetSegmentsCount = (int) mWaterJetParts.size() - 1;
    float jetMaxLength = GetCarPartLength(GameObjectType_WaterJet) * jetSegmentsCount + GetCarPartLength(GameObjectType_WaterJetEnd) * 0.5f;

    glm::vec2 jetStart = GetTurretMuzzlePosition();
    glm::vec2 jetEnd = jetStart + mTurret->mTransform.GetDirectionVector() * jetMaxLength;

    glm::vec2 wallPoint;
    if (gGameMap.TraceSegment2D(Convert::MetersToMapUnits(jetStart), Convert::MetersToMapUnits(jetEnd), Convert::MetersToMapUnits(mTransform.mPosition.y), wallPoint))
    {
        jetEnd = Convert::MapUnitsToMeters(wallPoint);
    }

    PhysicsQueryResult queryResults;
    gPhysics.QueryObjectsLinecast(jetStart, jetEnd, queryResults, CollisionGroup_Car | CollisionGroup_Pedestrian);

    Vehicle* hitCar = nullptr;
    for (int icurr = 0; icurr < queryResults.mElementsCount; ++icurr)
    {
        const PhysicsQueryElement& currElement = queryResults.mElements[icurr];
        Vehicle* car = ToVehicle(currElement.mPhysicsObject->mGameObject);
        if ((car == nullptr) || (car == this))
            continue;

        if (glm::distance(jetStart, currElement.mIntersectionPoint) < glm::distance(jetStart, jetEnd))
        {
            jetEnd = currElement.mIntersectionPoint;
            hitCar = car;
        }
    }

    if (hitCar)
    {
        hitCar->SetBurnEffectActive(false);
    }

    float jetLength = glm::distance(jetStart, jetEnd);
    SetWaterJetLength(jetLength);

    for (int icurr = 0; icurr < queryResults.mElementsCount; ++icurr)
    {
        const PhysicsQueryElement& currElement = queryResults.mElements[icurr];
        Pedestrian* pedestrian = ToPedestrian(currElement.mPhysicsObject->mGameObject);
        if ((pedestrian == nullptr) || (glm::distance(jetStart, currElement.mIntersectionPoint) > jetLength))
            continue;

        if (pedestrian->IsCarPassenger() || pedestrian->IsDead() || pedestrian->IsStunned() || !pedestrian->IsOnTheGround())
            continue;

        DamageInfo damageInfo;
        damageInfo.SetWaterJetDamage(mTurret);
        pedestrian->ReceiveDamage(damageInfo);
    }
}

void Vehicle::FireTankGun(Pedestrian* carDriver)
{
    WeaponInfo& weaponInfo = gGameMap.mStyleData.mWeaponTypes[eWeapon_RocketLauncher];
    if (gTimeManager.mGameTime < (mTankGunLastFireTime + (1.0f / weaponInfo.mBaseFireRate)))
        return;

    mTankGunLastFireTime = gTimeManager.mGameTime;

    glm::vec2 muzzlePosition = GetTurretMuzzlePosition();
    glm::vec3 projectilePosition(muzzlePosition.x, mTransform.mPosition.y, muzzlePosition.y);
    Projectile* projectile = gGameObjectsManager.CreateProjectile(projectilePosition, mTurret->mTransform.mOrientation, &weaponInfo, carDriver);
    debug_assert(projectile);

    if (weaponInfo.mShotSound != -1)
    {
        StartGameObjectSound(eCarSfxChannelIndex_Weapon, eSfxSampleType_Level, weaponInfo.mShotSound, SfxFlags_RandomPitch);
    }
    gBroadcastEvents.RegisterEvent(eBroadcastEvent_GunShot, this, carDriver, gGameParams.mBroadcastGunShotEventDuration);
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

int Vehicle::GetMaxDamage() const
{
    // original game does not use damagable value from style data
    return 100;
}

void Vehicle::AddDamage(int damage)
{
    if (IsCriticalDamageState())
        return;

    mCurrentDamage = std::min(mCurrentDamage + damage, GetMaxDamage());
}

int Vehicle::ComputeCollisionDamage(const Vehicle* otherCar) const
{
    // only hits with player car do damage
    Pedestrian* carDriver = GetCarDriver();
    Pedestrian* otherCarDriver = otherCar->GetCarDriver();
    bool isPlayerHit = (carDriver && carDriver->IsHumanPlayerCharacter()) || (otherCarDriver && otherCarDriver->IsHumanPlayerCharacter());
    if (!isPlayerHit || (mCarInfo->mModelID == eVehicle_Tank))
        return 0;

    // difference of momentums, velocity is along heading with integer speed
    auto getMomentum = [](const Vehicle* car)
    {
        int speed = (int) Convert::MetersPerSecondToPixelsPerTick(glm::length(car->mStepStartVelocity));
        glm::vec2 velocity = glm::floor(car->mTransform.GetDirectionVector() * (float) speed);
        return car->mCarInfo->mMass * velocity;
    };
    glm::vec2 momentum = getMomentum(this) - getMomentum(otherCar);
    int impactEnergy = ((int) std::fabs(momentum.x) + (int) std::fabs(momentum.y)) / 4;
    if (impactEnergy < 1)
        return 0;

    float damage = impactEnergy / (2.0f * mCarInfo->mMass);
    if (otherCar->mCarInfo->mModelID == eVehicle_Tank)
    {
        damage = std::max(damage, 10.0f);
    }
    return (int) damage;
}

void Vehicle::UpdateMapContactDamage()
{
    // wall hit damages car when it moves away from wall
    if (mPrevMapContact && !mMapContact && !IsWrecked())
    {
        AddDamage(GetSpeedPixels() / 4);
    }
    mPrevMapContact = mMapContact;
    mMapContact = false;
}

void Vehicle::UpdateFallDamage()
{
    if (!IsInAir())
        return;

    bool isCornerOnSlope = HasCornerOnSlope();

    // car goes up while it leaves ramp
    if ((mPhysicsBody->mFallHoverTime > 0.0f) && isCornerOnSlope)
    {
        mPhysicsBody->mPositionY += CarRampRiseSpeed * gPhysics.GetSimulationStepTime();
    }

    float fallDistance = mFallPrevHeight - mPhysicsBody->mPositionY;
    mFallPrevHeight = mPhysicsBody->mPositionY;

    if (IsWrecked() || IsCriticalDamageState())
        return;

    // 1 damage for each pixel of fall, but not above slope
    if ((fallDistance > 0.0f) && !isCornerOnSlope)
    {
        mFallDamagePixels += fallDistance / MAP_SPRITE_SCALE;
        int fallDamage = (int) mFallDamagePixels;
        mFallDamagePixels -= fallDamage;
        AddDamage(fallDamage);
    }

    // damage in air stops at 50, landing doubles it
    mCurrentDamage = std::min(mCurrentDamage, 50);
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
            damageInfo.SetElectricityDamage();
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
    return mCurrentDamage >= GetMaxDamage();
}

void Vehicle::UpdateEngineSound()
{
    if (mSfxEmitter == nullptr)
    {
        InitSounds();
        if (mSfxEmitter == nullptr)
            return;
    }

    if (IsWrecked() || !HasPassengers())
    {
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Engine);
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Skid);
        return;
    }

    int speed = GetSpeedPixels();
    UpdateSkidSound(speed);

    SfxSampleIndex sfxIndex = SfxLevel_FirstCarEngineSound + mCarInfo->mEngine;
    int sampleRate = GetCarEngineSampleRate(mCarInfo, sfxIndex, speed);
    if (sampleRate == 0)
    {
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Engine);
        return;
    }

    if (!mSfxEmitter->IsPlaying(eCarSfxChannelIndex_Engine))
    {
        if (!StartGameObjectSound(eCarSfxChannelIndex_Engine, eSfxSampleType_Level, sfxIndex, SfxFlags_Loop))
            return;
    }

    // player car engine keeps full gain when camera moves away
    Pedestrian* carDriver = GetCarDriver();
    mSfxEmitter->SetIgnoreDistance(eCarSfxChannelIndex_Engine, carDriver && carDriver->IsHumanPlayerCharacter());

    // engine runs faster when wheels are in air
    if (IsInAir())
    {
        sampleRate *= 3;
    }
    SetCarSoundSampleRate(mSfxEmitter, eCarSfxChannelIndex_Engine, sfxIndex, sampleRate);
    mSfxEmitter->SetGain(eCarSfxChannelIndex_Engine, GetCarSoundGain(speed));
}

void Vehicle::UpdateSkidSound(int speed)
{
    int skidValue = (int) mSkidValue;
    bool isHandBrakeSkid = mCtlState.mHandBrake && (speed > 1);
    bool isSkidding = false;
    int sampleRate = 0;
    if (mCarInfo->mSoundFunction == 5) // heavy vehicles
    {
        isSkidding = ((speed > 14) && (skidValue > 40)) || isHandBrakeSkid;
        sampleRate = (skidValue / 2) * 150 + 8000;
    }
    else if (mCarInfo->mSoundFunction != 4) // tank does not skid
    {
        isSkidding = (skidValue > 12) || isHandBrakeSkid;
        sampleRate = skidValue * 150 + 8000;
    }

    if (!isSkidding)
    {
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Skid);
        return;
    }

    if (!mSfxEmitter->IsPlaying(eCarSfxChannelIndex_Skid))
    {
        if (!StartGameObjectSound(eCarSfxChannelIndex_Skid, eSfxSampleType_Level, SfxLevel_CarSkid, SfxFlags_Loop))
            return;
    }
    SetCarSoundSampleRate(mSfxEmitter, eCarSfxChannelIndex_Skid, SfxLevel_CarSkid, sampleRate);
    mSfxEmitter->SetGain(eCarSfxChannelIndex_Skid, GetCarSoundGain(speed));
}

void Vehicle::UpdateSkidMarks()
{
    bool isSkidMarkVehicle = (mCarInfo->mClassID == eVehicleClass_StandardCar) || (mCarInfo->mClassID == eVehicleClass_Motorcycle);
    if (!isSkidMarkVehicle || (mSkidValue <= 20.0f) || IsInAir())
    {
        mSkidMarkTime = 0.0f;
        mBloodSkidMarks[0] = mBloodSkidMarks[1] = 0;
        return;
    }

    mSkidMarkTime -= gPhysics.GetSimulationStepTime();
    if (mSkidMarkTime > 0.0f)
        return;

    mSkidMarkTime += 1.0f / GTA_TICKS_PER_SECOND;

    glm::vec2 velocity = mPhysicsBody->GetLinearVelocity();
    cxx::angle_t heading = mTransform.mOrientation;
    if (glm::length(velocity) > 0.0f)
    {
        heading = cxx::angle_t::from_radians(::atan2f(velocity.y, velocity.x));
    }

    float rearOffset = -mCarInfo->mDimensions.z * 0.5f;
    if (mCarInfo->mClassID == eVehicleClass_Motorcycle)
    {
        CreateSkidMark(LocalForwardVector * rearOffset, heading, mBloodSkidMarks[0]);
        return;
    }

    float sideOffset = mCarInfo->mDimensions.x * 0.25f;
    CreateSkidMark(LocalForwardVector * rearOffset + LocalLateralVector * sideOffset, heading, mBloodSkidMarks[0]);
    CreateSkidMark(LocalForwardVector * rearOffset - LocalLateralVector * sideOffset, heading, mBloodSkidMarks[1]);
}

void Vehicle::CreateSkidMark(const glm::vec2& localPosition, cxx::angle_t heading, int& bloodSkidMarks)
{
    const int MaxSkidMarksInBlock = 17;
    const int MaxTyreSmokeInBlock = 3;

    glm::vec2 position2 = mPhysicsBody->GetWorldPoint(localPosition);
    glm::vec3 position(position2.x, mTransform.mPosition.y, position2.y);
    glm::ivec3 mapBlock = Convert::MetersToMapUnits(position);
    const MapBlockInfo* blockInfo = gGameMap.GetBlockInfo(mapBlock.x, mapBlock.z, mapBlock.y);
    // ground type is in block above block with lid graphic
    const MapBlockInfo* lidBlockInfo = gGameMap.GetBlockInfo(mapBlock.x, mapBlock.z, mapBlock.y - 1);

    SkidMarkArea area = ScanSkidMarkArea(position, glm::ivec2(mapBlock.x, mapBlock.z));
    if ((blockInfo->mGroundType == eGroundType_Road) && (area.mBlockTyreSmokeCount < MaxTyreSmokeInBlock))
    {
        gGameObjectsManager.CreateDecoration(position, heading, &gGameMap.mStyleData.mObjects[GameObjectType_TyreSmoke]);
    }

    int objectType = GameObjectType_SkidMark;
    if (bloodSkidMarks > 0)
    {
        objectType = GameObjectType_BloodSkidMark;
        --bloodSkidMarks;
    }
    else if (IsDirtSkidMarkLid(lidBlockInfo->mFaces[eBlockFace_Lid]))
    {
        objectType = GameObjectType_DirtSkidMark;
    }

    if (area.mIsBloodNear)
    {
        bloodSkidMarks = gCarnageGame.mGameRand.generate_int(9) + 12;
    }

    bool isSkidMarkGround = (blockInfo->mGroundType == eGroundType_Road) ||
        (blockInfo->mGroundType == eGroundType_Pawement) ||
        (blockInfo->mGroundType == eGroundType_Field);
    if (!isSkidMarkGround || (blockInfo->mSlopeType != 0) || (area.mBlockSkidMarksCount >= MaxSkidMarksInBlock) ||
        ((gGameParams.mCarSkidMarksMaxCount > 0) && (gGameObjectsManager.mSkidMarksCount >= gGameParams.mCarSkidMarksMaxCount)))
        return;

    Decoration* skidMark = gGameObjectsManager.CreateDecoration(position, heading, &gGameMap.mStyleData.mObjects[objectType]);
    if (skidMark)
    {
        skidMark->SetDrawOrder(eSpriteDrawOrder_GroundDecals);
    }
}

void Vehicle::UpdateHorn()
{
    const int SirenHornValue = 127;
    const int HornSoundsCount = SfxLevel_CarAlarm - SfxLevel_FirstCarHornSound;

    Pedestrian* carDriver = IsWrecked() ? nullptr : GetCarDriver();
    if (carDriver == nullptr)
    {
        EnableEmergencyLights(false);
    }

    if (mSfxEmitter == nullptr)
        return;

    bool isSoundActive = false;
    SfxSampleIndex sfxIndex = 0;
    if (mCarInfo->mHorn == SirenHornValue)
    {
        isSoundActive = IsEmergencyLightsEnabled();
        sfxIndex = (mCarInfo->mModelID == eVehicle_Ambulance) ? SfxLevel_AmbulanceSiren : SfxLevel_PoliceSiren;
    }
    else if (carDriver && carDriver->GetCtlState().mHorn && (mTurret == nullptr))
    {
        isSoundActive = true;
        sfxIndex = SfxLevel_FirstCarHornSound + (mCarInfo->mHorn % HornSoundsCount);
    }

    if (!isSoundActive)
    {
        mSfxEmitter->StopSound(eCarSfxChannelIndex_Misc);
        return;
    }

    if (!mSfxEmitter->IsPlaying(eCarSfxChannelIndex_Misc))
    {
        StartGameObjectSound(eCarSfxChannelIndex_Misc, eSfxSampleType_Level, sfxIndex, SfxFlags_Loop);
    }
}

int Vehicle::GetSpeedPixels() const
{
    return (int) Convert::MetersPerSecondToPixelsPerTick(glm::length(mPhysicsBody->GetLinearVelocity()));
}

float Vehicle::GetCurrentThrust() const
{
    if (mCurrentDamage > 25)
        return (125 - std::min(mCurrentDamage, 100)) * 0.01f * mCarInfo->mThrust;

    return mCarInfo->mThrust;
}

bool Vehicle::IsInAir() const
{
    return mPhysicsBody->mFalling;
}

bool Vehicle::HasCornerOnSlope() const
{
    glm::vec2 corners[4];
    GetChassisCorners(corners);

    int layer = (int) (Convert::MetersToMapUnits(mPhysicsBody->mPositionY) + 0.01f);
    for (const glm::vec2& currCorner: corners)
    {
        glm::ivec2 blockPosition = Convert::MetersToMapUnits(currCorner);
        const MapBlockInfo* blockInfo = gGameMap.GetBlockInfo(blockPosition.x, blockPosition.y, layer);
        if (blockInfo->mSlopeType > 0)
            return true;
    }
    return false;
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
    // front tyres keep their direction in world space when car turns
    float timeDelta = gPhysics.GetSimulationStepTime();
    float bodyAngle = mPhysicsBody->GetOrientation().to_radians();

    if (currCtlState.mSteerDirection != 0.0f)
    {
        if (!IsInAir())
        {
            // style data has turn ratio in degrees per tick
            float turnSpeed = glm::radians(1.0f * mCarInfo->mTurnRatio) * GTA_TICKS_PER_SECOND;
            mSteeringAngleWorld += turnSpeed * currCtlState.mSteerDirection * timeDelta;
        }
    }
    else
    {
        float steeringSin = std::sin(mSteeringAngleWorld - bodyAngle);
        if (steeringSin >= CarSteeringCenterAngle)
        {
            mSteeringAngleWorld -= CarSteeringRecenterSpeed * timeDelta;
        }
        else if (steeringSin < -CarSteeringCenterAngle)
        {
            mSteeringAngleWorld += CarSteeringRecenterSpeed * timeDelta;
        }

        // road assist turns player car to nearest road direction
        Pedestrian* carDriver = GetCarDriver();
        if (carDriver && carDriver->IsHumanPlayerCharacter() && (currCtlState.mDriveDirection >= 0.0f) && (GetSpeedPixels() > 6))
        {
            glm::ivec3 logPosition = Convert::MetersToMapUnits(mTransform.mPosition);
            const MapBlockInfo* blockInfo = gGameMap.GetBlockInfo(logPosition.x, logPosition.z, logPosition.y);
            if (blockInfo->mGroundType == eGroundType_Road)
            {
                float axisAngle = glm::round(bodyAngle / glm::half_pi<float>()) * glm::half_pi<float>();
                float axisOffset = bodyAngle - axisAngle;
                if (std::fabs(axisOffset) < CarRoadAssistLockAngle)
                {
                    bodyAngle = axisAngle;
                    mPhysicsBody->SetOrientation(cxx::angle_t::from_radians(axisAngle));
                    mPhysicsBody->SetAngularVelocity(cxx::angle_t());
                    mSteeringAngleWorld = axisAngle;
                }
                else if (std::fabs(axisOffset) < CarRoadAssistAngle)
                {
                    mSteeringAngleWorld -= glm::sign(axisOffset) * CarRoadAssistSpeed * timeDelta;
                }
            }
        }
    }

    mSteeringAngleRadians = std::remainder(mSteeringAngleWorld - bodyAngle, glm::two_pi<float>());
    mSteeringAngleRadians = glm::clamp(mSteeringAngleRadians, -CarSteeringLockAngle, CarSteeringLockAngle);
    mSteeringAngleWorld = bodyAngle + mSteeringAngleRadians;
}

void Vehicle::UpdateTyreForces(const DriveCtlState& currCtlState)
{
    mSkidValue = 0.0f;

    if (IsInAir())
        return;

    float driveForce = 0.0f;
    if (currCtlState.mDriveDirection > 0.0f)
    {
        driveForce = GetCurrentThrust() * currCtlState.mDriveDirection;
        if ((mCarInfo->mModelID == eVehicle_Tank) && (GetSpeedPixels() > mCarInfo->mMaxSpeed))
        {
            driveForce = 0.0f;
        }
    }
    else if (currCtlState.mDriveDirection < 0.0f)
    {
        float reverseCoef = (mCarInfo->mClassID == eVehicleClass_Motorcycle) ? 0.15f : 0.5f;
        driveForce = GetCurrentThrust() * reverseCoef * currCtlState.mDriveDirection;
    }

    // rear tyres have more grip, hand brake makes them slide
    bool isCoach = (mCarInfo->mModelID == eVehicle_Coach);
    float rearCoef = isCoach ? CoachRearTyreCoef : CarRearTyreCoef;
    float rearForwardFriction = mCarInfo->mTyreAdhesionX * rearCoef;
    float rearLateralFriction = mCarInfo->mTyreAdhesionY * rearCoef;
    if (currCtlState.mHandBrake)
    {
        rearForwardFriction = (mCarInfo->mHandbrakeFriction + mCarInfo->mTyreAdhesionX) * rearCoef;
        rearLateralFriction = mCarInfo->mTyreAdhesionY * (isCoach ? CoachRearTyreCoef : CarHandBrakeLateralCoef);
    }

    float lateralForce = ApplyTyreForce(eCarTire_Front, mCarInfo->mTyreAdhesionX, mCarInfo->mTyreAdhesionY, 0.0f);
    lateralForce += ApplyTyreForce(eCarTire_Rear, rearForwardFriction, rearLateralFriction, driveForce);
    mSkidValue = std::fabs(lateralForce);

    // hand brake with gas always makes skid
    if (currCtlState.mHandBrake && (currCtlState.mDriveDirection > 0.0f) && (GetSpeedPixels() > 2))
    {
        mSkidValue = 21.0f;
    }
}

float Vehicle::ApplyTyreForce(eCarTire tireID, float forwardFriction, float lateralFriction, float driveForce)
{
    glm::vec2 tirePosition = GetTirePosition(tireID);
    glm::vec2 tireForward = GetTireForward(tireID);
    glm::vec2 tireLateral = GetTireLateral(tireID);

    // tyre forces are in pixels per tick and do not use mass
    float metersPerSecond = Convert::PixelsPerTickToMetersPerSecond(1.0f);
    glm::vec2 tireVelocity = mPhysicsBody->GetLinearVelocityFromWorldPoint(tirePosition) / metersPerSecond;
    float forwardForce = driveForce - forwardFriction * glm::dot(tireVelocity, tireForward);
    float lateralForce = -lateralFriction * glm::dot(tireVelocity, tireLateral);

    float forceScale = CarMassScale * metersPerSecond * GTA_TICKS_PER_SECOND;
    mPhysicsBody->AddForce((tireForward * forwardForce + tireLateral * lateralForce) * forceScale, tirePosition);
    return lateralForce;
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
