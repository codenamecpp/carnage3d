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
    
    mPhysicsComponent = gPhysics.CreateCarPhysicsComponent(this, startPosition, cxx::angle_t::from_degrees(0.0f), mCarStyle);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
    mDamageDeltaBits = 0;
    mChassisSpriteIndex = gGameMap.mStyleData.GetCarSpriteIndex(mCarStyle->mVType, mCarStyle->mSprNum);

    SetupDeltaAnimations();
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
    UpdateDeltaAnimations(deltaTime);
}

void Vehicle::DrawFrame(SpriteBatch& spriteBatch)
{   
    cxx::angle_t rotationAngle = mPhysicsComponent->GetRotationAngle() - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);

    glm::vec3 position = mPhysicsComponent->GetPosition();
    position.y = ComputeDrawHeight(position, rotationAngle);

    gSpriteManager.GetSpriteTexture(mObjectID, mChassisSpriteIndex, GetSpriteDeltas(), mChassisDrawSprite);
    mChassisDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mChassisDrawSprite.mScale = SPRITE_SCALE;
    mChassisDrawSprite.mRotateAngle = rotationAngle;
    mChassisDrawSprite.mHeight = ComputeDrawHeight(position, rotationAngle);
    mChassisDrawSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mChassisDrawSprite);

#if 1 // debug
    // draw doors
    for (int idoor = 0; idoor < mCarStyle->mDoorsCount; ++idoor)
    {
        float x = position.x + (1.0f * mCarStyle->mDoors[idoor].mRpx / MAP_PIXELS_PER_TILE);
        float z = position.z + (1.0f * mCarStyle->mDoors[idoor].mRpy / MAP_PIXELS_PER_TILE);
        glm::vec2 rotated_pos = cxx::rotate_around_center(glm::vec2(x, z), glm::vec2(position.x, position.z), rotationAngle);
        gRenderManager.mDebugRenderer.DrawCube(glm::vec3(rotated_pos.x, position.y + 0.05f, rotated_pos.y), glm::vec3(0.05f, 0.05f, 0.05f), COLOR_RED);
    }
#endif
}

float Vehicle::ComputeDrawHeight(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    float halfW = (1.0f * mCarStyle->mWidth) / MAP_BLOCK_TEXTURE_DIMS * 0.5f;
    float halfH = (1.0f * mCarStyle->mHeight) / MAP_BLOCK_TEXTURE_DIMS * 0.5f;

    glm::vec3 points[4] = {
        { -halfW, position.y + 0.01f, -halfH },
        { halfW,  position.y + 0.01f, -halfH },
        { halfW,  position.y + 0.01f, halfH },
        { -halfW, position.y + 0.01f, halfH },
    };

    float maxHeight = position.y;
    for (glm::vec3& currPoint: points)
    {
        currPoint = glm::rotate(currPoint, rotationAngle.to_radians(), glm::vec3(0.0f, -1.0f, 0.0f));
        currPoint.x += position.x;
        currPoint.z += position.z;
    }
#if 1 // debug
    for (int i = 0; i < 4; ++i)
    {
        gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_RED);
    }
#endif
    return position.y + 0.02f;
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