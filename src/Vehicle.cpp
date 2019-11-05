#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "Pedestrian.h"

Vehicle::Vehicle(GameObjectID id)
    : GameObject(eGameObjectType_Car, id)
    , mActiveCarsNode(this)
    , mDeleteCarsNode(this)
    , mPhysicsComponent()
    , mDead()
    , mCarStyle()
    , mDamageDeltaBits()
    , mDrawHeight()
{
}

Vehicle::~Vehicle()
{
    debug_assert(mPassengers.empty());
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
    gSpriteManager.FlushSpritesCache(mObjectID);
}

void Vehicle::EnterTheGame(const glm::vec3& startPosition, cxx::angle_t startRotation)
{
    debug_assert(mCarStyle);
    
    mPhysicsComponent = gPhysics.CreatePhysicsComponent(this, startPosition, startRotation, mCarStyle);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
    mDamageDeltaBits = 0;
    mChassisSpriteIndex = gGameMap.mStyleData.GetCarSpriteIndex(mCarStyle->mVType, mCarStyle->mSprNum); // todo: handle bike fallen state 
    mRemapIndex = NO_REMAP;

    SetupDeltaAnimations();
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
    UpdateDeltaAnimations(deltaTime);

    UpdateDriving(deltaTime);
}

void Vehicle::DrawFrame(SpriteBatch& spriteBatch)
{   
    // sync sprite transformation with physical body
    cxx::angle_t rotationAngle = mPhysicsComponent->GetRotationAngle() - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    glm::vec3 position = mPhysicsComponent->GetPosition();
    ComputeDrawHeight(position);

    int remapClut = mRemapIndex == NO_REMAP ? 0 : (mCarStyle->mRemapsBaseIndex + mRemapIndex);
    gSpriteManager.GetSpriteTexture(mObjectID, mChassisSpriteIndex, remapClut, GetSpriteDeltas(), mChassisDrawSprite);

    mChassisDrawSprite.mPosition.x = position.x;
    mChassisDrawSprite.mPosition.y = position.z;
    mChassisDrawSprite.mScale = SPRITE_SCALE;
    mChassisDrawSprite.mRotateAngle = rotationAngle;
    mChassisDrawSprite.mHeight = mDrawHeight;
    mChassisDrawSprite.SetOriginToCenter();

    spriteBatch.DrawSprite(mChassisDrawSprite);

#if 0
    DrawDebug();
#endif
}

void Vehicle::DrawDebug()
{
    glm::vec3 position = mPhysicsComponent->GetPosition();

    glm::vec2 corners[4];
    mPhysicsComponent->GetChassisCorners(corners);

    glm::vec3 points[4];
    for (int i = 0; i < 4; ++i)
    {
        points[i].x = corners[i].x;
        points[i].z = corners[i].y;
        points[i].y = mDrawHeight;
    }
    for (int i = 0; i < 4; ++i)
    {
        gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_RED);
    }

    // draw doors
    for (int idoor = 0; idoor < mCarStyle->mDoorsCount; ++idoor)
    {
        glm::vec2 rotated_pos;
        GetDoorPos(idoor, rotated_pos);
        gRenderManager.mDebugRenderer.DrawCube(glm::vec3(rotated_pos.x, position.y + 0.05f, rotated_pos.y), glm::vec3(0.05f, 0.05f, 0.05f), COLOR_YELLOW);
    }

    if (mCarStyle->mDoorsCount > 0)
    {
        glm::vec2 seatpos;
        GetSeatPos(eCarSeat_Driver, seatpos);
        gRenderManager.mDebugRenderer.DrawCube(glm::vec3(seatpos.x, position.y + 0.05f, seatpos.y), glm::vec3(0.05f, 0.05f, 0.05f), COLOR_GREEN);
    }

    // draw wheels
    for (eCarWheelID currID: {eCarWheelID_Drive, eCarWheelID_Steering})
    {
        if (!mPhysicsComponent->HasWheel(currID))
            continue;

        mPhysicsComponent->GetWheelCorners(currID, corners);

        for (int i = 0; i < 4; ++i)
        {
            points[i].x = corners[i].x;
            points[i].z = corners[i].y;
            points[i].y = mDrawHeight;
        }
        for (int i = 0; i < 4; ++i)
        {
            gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_YELLOW);
        }
        glm::vec2 forwardVelocity = mPhysicsComponent->GetWheelForwardVelocity(currID);
        glm::vec2 lateralVelocity = mPhysicsComponent->GetWheelLateralVelocity(currID);
        glm::vec2 wheelPosition = mPhysicsComponent->GetWheelPosition(currID);

        gRenderManager.mDebugRenderer.DrawLine(
            glm::vec3 {wheelPosition.x, mDrawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + forwardVelocity.x, mDrawHeight, wheelPosition.y + forwardVelocity.y}, COLOR_GREEN);
        gRenderManager.mDebugRenderer.DrawLine(
            glm::vec3 {wheelPosition.x, mDrawHeight, wheelPosition.y},
            glm::vec3 {wheelPosition.x + lateralVelocity.x, mDrawHeight, wheelPosition.y + lateralVelocity.y}, COLOR_SKYBLUE);
    }
}

void Vehicle::ComputeDrawHeight(const glm::vec3& position)
{
    glm::vec2 corners[4];
    mPhysicsComponent->GetChassisCorners(corners);

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

    mDrawHeight = maxHeight + 0.02f; // todo: magic numbers
}

SpriteDeltaBits Vehicle::GetSpriteDeltas() const
{
    SpriteDeltaBits deltaBits = mDamageDeltaBits;

    // add doors
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        if (!mDoorsAnims[idoor].IsNull())
        {
            unsigned int deltaBit = mDoorsAnims[idoor].GetCurrentFrame();
            deltaBits |= deltaBit;
        }
    }

    // add emergency lights
    if (mEmergLightsAnim.IsAnimationActive())
    {
        unsigned int deltaBit = mEmergLightsAnim.GetCurrentFrame();
        deltaBits |= deltaBit;
    }

    return deltaBits;
}

void Vehicle::SetupDeltaAnimations()
{
    SpriteDeltaBits deltaBits = gGameMap.mStyleData.mSprites[mChassisSpriteIndex].GetDeltaBits();

    mEmergLightsAnim.SetNull();
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        mDoorsAnims[idoor].SetNull();
    }

    SpriteDeltaBits maskBits = BIT(CAR_LIGHTING_SPRITE_DELTA_0) | BIT(CAR_LIGHTING_SPRITE_DELTA_1);
    if ((deltaBits & maskBits) == maskBits)
    {
        mEmergLightsAnim.SetNull();
        mEmergLightsAnim.mAnimData.SetupFrames(
        {
            BIT(CAR_LIGHTING_SPRITE_DELTA_0), BIT(CAR_LIGHTING_SPRITE_DELTA_0), BIT(CAR_LIGHTING_SPRITE_DELTA_0),
            BIT(CAR_LIGHTING_SPRITE_DELTA_1), BIT(CAR_LIGHTING_SPRITE_DELTA_1), BIT(CAR_LIGHTING_SPRITE_DELTA_1),
        }, 
        CAR_DELTA_ANIMS_SPEED);
    }

    // doors
    if (mCarStyle->mDoorsCount >= 1)
    {
        mDoorsAnims[0].mAnimData.SetupFrames(
        {
            0,
            BIT(CAR_DOOR1_SPRITE_DELTA_0),
            BIT(CAR_DOOR1_SPRITE_DELTA_1),
            BIT(CAR_DOOR1_SPRITE_DELTA_2),
            BIT(CAR_DOOR1_SPRITE_DELTA_3)
        }, 
        CAR_DELTA_ANIMS_SPEED);
    }

    if (mCarStyle->mDoorsCount >= 2)
    {
        mDoorsAnims[1].mAnimData.SetupFrames(
        {
            0,
            BIT(CAR_DOOR2_SPRITE_DELTA_0),
            BIT(CAR_DOOR2_SPRITE_DELTA_1),
            BIT(CAR_DOOR2_SPRITE_DELTA_2),
            BIT(CAR_DOOR2_SPRITE_DELTA_3)
        }, 
        CAR_DELTA_ANIMS_SPEED);
    }
}

void Vehicle::UpdateDeltaAnimations(Timespan deltaTime)
{
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        if (mDoorsAnims[idoor].IsAnimationActive())
        {
            mDoorsAnims[idoor].AdvanceAnimation(deltaTime);
        }
    }

    if (mEmergLightsAnim.IsAnimationActive())
    {
        mEmergLightsAnim.AdvanceAnimation(deltaTime);
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

        mDoorsAnims[doorIndex].PlayAnimationBackwards(eSpriteAnimLoop_None);
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
    return HasDoorAnimation(doorIndex) && !mDoorsAnims[doorIndex].IsAnimationActive() && 
        mDoorsAnims[doorIndex].IsLastFrame();
}

bool Vehicle::IsDoorClosed(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && !mDoorsAnims[doorIndex].IsAnimationActive() && 
        mDoorsAnims[doorIndex].IsFirstFrame();
}

bool Vehicle::IsDoorOpening(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && mDoorsAnims[doorIndex].IsAnimationActive() && 
        mDoorsAnims[doorIndex].IsRunsForwards();
}

bool Vehicle::IsDoorClosing(int doorIndex) const
{
    debug_assert(doorIndex < MAX_CAR_DOORS);
    return HasDoorAnimation(doorIndex) && mDoorsAnims[doorIndex].IsAnimationActive() && 
        mDoorsAnims[doorIndex].IsRunsBackwards();
}

bool Vehicle::HasEmergencyLightsAnimation() const
{
    return !mEmergLightsAnim.IsNull();
}

bool Vehicle::IsEmergencyLightsEnabled() const
{
    return HasEmergencyLightsAnimation() && mEmergLightsAnim.IsAnimationActive();
}

void Vehicle::EnableEmergencyLights(bool isEnabled)
{
    if (HasEmergencyLightsAnimation())
    {
        if (isEnabled == mEmergLightsAnim.IsAnimationActive())
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
    if (doorIndex > -1 && doorIndex < mCarStyle->mDoorsCount)
    {
        out.x = ConvertPixelsToMap(mCarStyle->mDoors[doorIndex].mRpy);
        out.y = -ConvertPixelsToMap(mCarStyle->mDoors[doorIndex].mRpx);
        return true;
    }

    debug_assert(false);
    return false;
}

bool Vehicle::GetDoorPos(int doorIndex, glm::vec2& out) const
{
    if (GetDoorPosLocal(doorIndex, out))
    {
        out = mPhysicsComponent->GetWorldPoint(out);
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
        bool isBike = (mCarStyle->mVType == eCarVType_Motorcycle);
        if (isBike)
        {
            out.y = 0.0f; // dead center
        }
        else
        {
            out.y = -ConvertPixelsToMap(mCarStyle->mWidth) * 0.25f * -glm::sign(doorLocalPos.y);
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
        out = mPhysicsComponent->GetWorldPoint(out);
        return true;
    }

    debug_assert(false);
    return false;
}

bool Vehicle::IsSeatPresent(eCarSeat carSeat) const
{
    int doorIndex = GetDoorIndexForSeat(carSeat);
    return (doorIndex < mCarStyle->mDoorsCount && doorIndex > -1);
}

void Vehicle::PutPassenger(Pedestrian* pedestrian, eCarSeat carSeat)
{
    if (pedestrian == nullptr || carSeat == eCarSeat_Any)
    {
        debug_assert(false);
        return;
    }
    // check is already added
    if (std::find(mPassengers.begin(), mPassengers.end(), pedestrian) == mPassengers.end())
    {
        mPassengers.push_back(pedestrian);
    }
    else
    {
        debug_assert(false);
    }
}

void Vehicle::RemovePassenger(Pedestrian* pedestrian)
{
    auto ifound = std::find(mPassengers.begin(), mPassengers.end(), pedestrian);
    if (ifound != mPassengers.end())
    {
        mPassengers.erase(ifound);
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

void Vehicle::UpdateDriving(Timespan deltaTime)
{
    Pedestrian* carDriver = GetCarDriver();
    if (carDriver == nullptr)
    {
        mPhysicsComponent->ResetDriveState();
        return;
    }

    mPhysicsComponent->SetHandBrake(carDriver->mCtlActions[ePedestrianAction_HandBrake]);
    mPhysicsComponent->SetAcceleration(carDriver->mCtlActions[ePedestrianAction_Accelerate]);
    mPhysicsComponent->SetDeceleration(carDriver->mCtlActions[ePedestrianAction_Reverse]);

    // steering
    int currentSteering = CarSteeringDirectionNone;
    if (carDriver->mCtlActions[ePedestrianAction_SteerLeft])
    {
        currentSteering = CarSteeringDirectionLeft;
    }
    if (carDriver->mCtlActions[ePedestrianAction_SteerRight])
    {
        currentSteering = CarSteeringDirectionRight;
    }
    mPhysicsComponent->SetSteering(currentSteering);
}
