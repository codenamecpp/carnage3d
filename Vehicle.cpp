#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "RenderingManager.h"
#include "SpriteManager.h"

Vehicle::Vehicle(unsigned int id)
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
    
    mPhysicsComponent = gPhysics.CreateCarPhysicsComponent(startPosition, 0.0f, mCarStyle);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
}

void Vehicle::DrawFrame(SpriteBatch& spriteBatch)
{
    int spriteLinearIndex = gGameMap.mStyleData.GetCarSpriteIndex(mCarStyle->mVType, mCarStyle->mModel, mCarStyle->mSprNum);
        
    float rotationAngle = glm::radians(mPhysicsComponent->GetAngleDegrees() - SPRITE_ZERO_ANGLE);

    glm::vec3 position = mPhysicsComponent->GetPosition();
    position.y = ComputeDrawHeight(position, rotationAngle);

    mChassisSprite.mTexture = gSpriteManager.mObjectsSpritesheet.mSpritesheetTexture;
    mChassisSprite.mTextureRegion = gSpriteManager.mObjectsSpritesheet.mEtries[spriteLinearIndex];
    mChassisSprite.mPosition = glm::vec2(position.x, position.z);
    mChassisSprite.mScale = SPRITE_SCALE;
    mChassisSprite.mRotateAngleRads = rotationAngle;
    mChassisSprite.mHeight = ComputeDrawHeight(position, rotationAngle);
    mChassisSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mChassisSprite);
}

float Vehicle::ComputeDrawHeight(const glm::vec3& position, float angleRadians)
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
        currPoint = glm::rotate(currPoint, angleRadians, glm::vec3(0.0f, -1.0f, 0.0f));
        currPoint.x += position.x;
        currPoint.z += position.z;
    }
#if 1
    // debug
    for (int i = 0; i < 4; ++i)
    {
        gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_RED);
    }
#endif
    return position.y + 0.02f;
}
