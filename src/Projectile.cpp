#include "stdafx.h"
#include "Projectile.h"
#include "PhysicsManager.h"
#include "SpriteManager.h"
#include "GameMapManager.h"
#include "TimeManager.h"
#include "DebugRenderer.h"

Projectile::Projectile(ProjectileStyle* style) 
    : GameObject(eGameObjectClass_Projectile, GAMEOBJECT_ID_NULL)
    , mProjectileStyle(style)
    , mDrawHeight()
    , mStartPosition()
{
    debug_assert(style);
}

Projectile::~Projectile()
{
    if (mPhysicsBody)
    {
        gPhysics.DestroyPhysicsBody(mPhysicsBody);
    }
}

void Projectile::Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation)
{
    debug_assert(mProjectileStyle);

    mStartPosition = startPosition;
    mDead = false;
    if (mPhysicsBody == nullptr)
    {
        mPhysicsBody = gPhysics.CreatePhysicsBody(this, startPosition, startRotation);
        debug_assert(mPhysicsBody);
    }
    else
    {   
        mPhysicsBody->SetPosition(startPosition, startRotation);
    }

    // setup animation
    mAnimationState.SetNull();
    if (!gGameMap.mStyleData.GetSpriteAnimation(mProjectileStyle->mAnimID, mAnimationState.mAnimDesc))
    {
        debug_assert(false);
    }
    mAnimationState.PlayAnimation(mProjectileStyle->mAnimLoop);

    // setup sprite rotation and scale
    cxx::angle_t rotationAngle = startRotation + cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    mDrawSprite.mRotateAngle = rotationAngle;
}

void Projectile::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    mAnimationState.AdvanceAnimation(deltaTime);
}

void Projectile::PreDrawFrame()
{
    int spriteLinearIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Object, mAnimationState.GetCurrentFrame());
    gSpriteManager.GetSpriteTexture(mObjectID, spriteLinearIndex, 0, mDrawSprite);

    glm::vec3 position = mPhysicsBody->mSmoothPosition;
    ComputeDrawHeight(position);

    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mDrawSprite.mHeight = mDrawHeight;
    mDrawSprite.SetOriginToCenter();
}

void Projectile::DrawDebug(DebugRenderer& debugRender)
{   
    cxx::bounding_sphere_t bsphere (mPhysicsBody->GetPosition(), mProjectileStyle->mProjectileRadius);
    debugRender.DrawSphere(bsphere, Color32_Orange, false);
}

void Projectile::ComputeDrawHeight(const glm::vec3& position)
{
    float maxHeight = position.y;

    glm::vec2 corners[4];
    mDrawSprite.GetCorners(corners);
    for (glm::vec2& currCorner: corners)
    {
        float height = gGameMap.GetHeightAtPosition(glm::vec3(currCorner.x, position.y, currCorner.y));
        if (height > maxHeight)
        {
            maxHeight = height;
        }
    }

    mDrawHeight = maxHeight + 0.1f; // todo: magic numbers
}
