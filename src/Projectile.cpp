#include "stdafx.h"
#include "Projectile.h"
#include "PhysicsManager.h"
#include "SpriteManager.h"
#include "GameMapManager.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "GameObjectsManager.h"

Projectile::Projectile() 
    : GameObject(eGameObjectClass_Projectile, GAMEOBJECT_ID_NULL)
    , mStartPosition()
{
}

Projectile::~Projectile()
{
    if (mPhysicsBody)
    {
        gPhysics.DestroyPhysicsBody(mPhysicsBody);
    }
}

void Projectile::Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation, WeaponInfo* weaponInfo)
{
    mWeaponInfo = weaponInfo;
    mStartPosition = startPosition;
    mContactDetected = false;
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
    mAnimationState.Clear();

    if (weaponInfo)
    {
        StyleData& cityStyle = gGameMap.mStyleData;

        int objectindex = weaponInfo->mProjectileObject;
        if (objectindex > 0 && objectindex < (int) cityStyle.mGameObjects.size())
        {
            GameObjectStyle& objectInfo = cityStyle.mGameObjects[objectindex];

            debug_assert(objectInfo.mClassID == eGameObjectClass_Projectile);
            
            mAnimationState.Clear();
            mAnimationState.mAnimDesc = objectInfo.mAnimationData;

            eSpriteAnimLoop loopType = eSpriteAnimLoop_FromStart;
            if (!weaponInfo->IsFireDamage()) // todo: move to configs
            {
                loopType = eSpriteAnimLoop_None;
            }
            mAnimationState.PlayAnimation(loopType, 24.0f); // todo: move to config
        }
    }

    // setup sprite rotation and scale
    cxx::angle_t rotationAngle = startRotation + cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    mDrawSprite.mRotateAngle = rotationAngle;
}

void Projectile::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    mAnimationState.AdvanceAnimation(deltaTime);

    if (IsContactDetected())
    {
        MarkForDeletion();

        if (mWeaponInfo)
        {
            if (mWeaponInfo->IsExplosionDamage())
            {
                Explosion* explosion = gGameObjectsManager.CreateExplosion(mContactPoint);
            }

            if (mWeaponInfo->mProjectileHitEffect > GameObjectType_Null)
            {
                GameObjectStyle& objectInfo = gGameMap.mStyleData.mGameObjects[mWeaponInfo->mProjectileHitEffect];
                Decoration* hitEffect = gGameObjectsManager.CreateDecoration(mContactPoint, cxx::angle_t(), &objectInfo);
                debug_assert(hitEffect);

                if (hitEffect)
                {
                    hitEffect->SetLifeDuration(1);
                }
            }
        }
    }
}

void Projectile::PreDrawFrame()
{
    int spriteIndex = mAnimationState.GetCurrentFrame();
    gSpriteManager.GetSpriteTexture(mObjectID, spriteIndex, 0, mDrawSprite);

    glm::vec3 position = mPhysicsBody->mSmoothPosition;
    ComputeDrawHeight(position);

    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mDrawSprite.SetOriginToCenter();
}

void Projectile::DrawDebug(DebugRenderer& debugRender)
{   
    if (mWeaponInfo)
    {
        cxx::bounding_sphere_t bsphere (mPhysicsBody->GetPosition(), mWeaponInfo->mProjectileSize);
        debugRender.DrawSphere(bsphere, Color32_Orange, false);
    }
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

    mDrawSprite.mHeight = maxHeight + 0.1f; // todo: magic numbers
}

void Projectile::SetContactDetected(const glm::vec3& position, GameObject* gameObject)
{
    mContactDetected = true;
    mContactObject = gameObject;
    mContactPoint = position;

    if (gameObject == nullptr)
    {
        mContactPoint.y += Convert::MapUnitsToMeters(1.0f); // todo: magic numbers
    }
    else 
    {
        mContactPoint.y += 0.1f; // todo: magic numbers
    }
}

bool Projectile::IsContactDetected() const
{
    return mContactDetected;
}
