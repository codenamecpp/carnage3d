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
{
}

Vehicle::~Vehicle()
{
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
}

void Vehicle::EnterTheGame()
{
    debug_assert(mCarStyle);

    glm::vec3 startPosition;
    
    mPhysicsComponent = gPhysics.CreateCarPhysicsComponent(this, startPosition, cxx::angle_t::from_degrees(0.0f), mCarStyle);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
}

void Vehicle::DrawFrame(SpriteBatch& spriteBatch)
{   
    cxx::angle_t rotationAngle = mPhysicsComponent->GetRotationAngle() - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);

    glm::vec3 position = mPhysicsComponent->GetPosition();
    position.y = ComputeDrawHeight(position, rotationAngle);

    int spriteLinearIndex = gGameMap.mStyleData.GetCarSpriteIndex(mCarStyle->mVType, mCarStyle->mModel, mCarStyle->mSprNum);
    gSpriteManager.GetSpriteTexture(mObjectID, spriteLinearIndex, GetSpriteDeltas(), mChassisSprite);

    mChassisSprite.mPosition = glm::vec2(position.x, position.z);
    mChassisSprite.mScale = SPRITE_SCALE;
    mChassisSprite.mRotateAngle = rotationAngle;
    mChassisSprite.mHeight = ComputeDrawHeight(position, rotationAngle);
    mChassisSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mChassisSprite);

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
    return 0;
}
