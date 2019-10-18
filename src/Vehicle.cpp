#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "RenderingManager.h"
#include "SpriteManager.h"

Vehicle::Vehicle(GameObjectID_t id)
    : GameObject(id)
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
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
    gSpriteManager.FlushSpritesCache(mObjectID);
}

void Vehicle::EnterTheGame()
{
    debug_assert(mCarStyle);

    glm::vec3 startPosition;
    
    mPhysicsComponent = gPhysics.CreatePhysicsComponent(this, startPosition, cxx::angle_t::from_degrees(0.0f), mCarStyle);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
    mIsOnScreen = false;
    mDamageDeltaBits = 0;
    mChassisSpriteIndex = gGameMap.mStyleData.GetCarSpriteIndex(mCarStyle->mVType, mCarStyle->mSprNum); // todo: handle bike fallen state 

    SetupDeltaAnimations();
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
    UpdateDeltaAnimations(deltaTime);    
}

void Vehicle::DrawFrame(SpriteBatch& spriteBatch)
{   
    // sync sprite transformation with physical body
    cxx::angle_t rotationAngle = mPhysicsComponent->GetRotationAngle() - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    glm::vec3 position = mPhysicsComponent->GetPosition();
    ComputeDrawHeight(position);

    gSpriteManager.GetSpriteTexture(mObjectID, mChassisSpriteIndex, GetSpriteDeltas(), mChassisDrawSprite);

    mChassisDrawSprite.mPosition.x = position.x;
    mChassisDrawSprite.mPosition.y = position.z;
    mChassisDrawSprite.mScale = SPRITE_SCALE;
    mChassisDrawSprite.mRotateAngle = rotationAngle;
    mChassisDrawSprite.mHeight = mDrawHeight;
    mChassisDrawSprite.SetOriginToCenter();

    // update is on screen
    glm::vec2 corners[4];
    mChassisDrawSprite.GetCorners(corners);

    glm::vec2 minpos = corners[0];
    glm::vec2 maxpos = corners[0];

    for (int icorner = 1; icorner < 4; ++icorner)
    {
        minpos = glm::min(minpos, corners[icorner]);
        maxpos = glm::max(maxpos, corners[icorner]);
    }

    float halfw = (maxpos.x - minpos.x) * 0.5f;
    float halfh = (maxpos.y - minpos.y) * 0.5f;

    cxx::aabbox_t aabox;
    aabox.mMin.x = mChassisDrawSprite.mPosition.x - halfw;
    aabox.mMin.y = mPhysicsComponent->mHeight - 0.1f;
    aabox.mMin.z = mChassisDrawSprite.mPosition.y - halfh;

    aabox.mMax.x = mChassisDrawSprite.mPosition.x + halfw;
    aabox.mMax.y = mPhysicsComponent->mHeight + 0.1f;
    aabox.mMax.z = mChassisDrawSprite.mPosition.y + halfh;

    mIsOnScreen = gCamera.mFrustum.contains(aabox);
    if (!mIsOnScreen)
        return;

    spriteBatch.DrawSprite(mChassisDrawSprite);

#if 1 // debug
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

    float bboxheight = (1.0f * mCarStyle->mHeight / MAP_PIXELS_PER_TILE);

    int bboxcolor = COLOR_WHITE;
    if (mCarStyle->mConvertible == eCarConvertible_HardTop || mCarStyle->mConvertible == eCarConvertible_HardTopAnimated)
    {
        bboxcolor = COLOR_GREEN;
    }

    gRenderManager.mDebugRenderer.DrawCube(glm::vec3(mChassisDrawSprite.mPosition.x, mPhysicsComponent->mHeight, mChassisDrawSprite.mPosition.y), 
        glm::vec3((maxpos.x - minpos.x), bboxheight, (maxpos.y - minpos.y)), bboxcolor);
#endif
}

void Vehicle::ComputeDrawHeight(const glm::vec3& position)
{
    glm::vec2 corners[4];
    mPhysicsComponent->GetChassisCorners(corners);

    float maxHeight = position.y;
#if 1 // debug
    glm::vec3 points[4];
    for (int i = 0; i < 4; ++i)
    {
        points[i].x = corners[i].x;
        points[i].z = corners[i].y;
        points[i].y = maxHeight;
    }
    for (int i = 0; i < 4; ++i)
    {
        gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_RED);
    }
#endif
    mDrawHeight = position.y + 0.02f; // todo: magic numbers
}

SpriteDeltaBits_t Vehicle::GetSpriteDeltas() const
{
    SpriteDeltaBits_t deltaBits = mDamageDeltaBits;

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
    SpriteDeltaBits_t deltaBits = gGameMap.mStyleData.mSprites[mChassisSpriteIndex].GetDeltaBits();

    mEmergLightsAnim.SetNull();
    for (int idoor = 0; idoor < MAX_CAR_DOORS; ++idoor)
    {
        mDoorsAnims[idoor].SetNull();
    }

    SpriteDeltaBits_t maskBits = BIT(CAR_LIGHTING_SPRITE_DELTA_0) | BIT(CAR_LIGHTING_SPRITE_DELTA_1);
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

    // todo: bike
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
        out.x = (1.0f * mCarStyle->mDoors[doorIndex].mRpy / MAP_PIXELS_PER_TILE);
        out.y = -(1.0f * mCarStyle->mDoors[doorIndex].mRpx / MAP_PIXELS_PER_TILE);
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
    // todo: handle bike

    int doorIndex = GetDoorIndexForSeat(carSeat);

    glm::vec2 doorLocalPos;
    if (GetDoorPosLocal(doorIndex, doorLocalPos))
    {
        out.x = doorLocalPos.x;
        out.y = -(1.0f * mCarStyle->mWidth / MAP_PIXELS_PER_TILE) * 0.25f;
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