#include "stdafx.h"
#include "Projectile.h"
#include "PhysicsManager.h"
#include "SpriteManager.h"
#include "GameMapManager.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "GameObjectsManager.h"
#include "AudioManager.h"

Projectile::Projectile(WeaponInfo* weaponInfo, Pedestrian* shooter) 
    : GameObject(eGameObjectClass_Projectile, GAMEOBJECT_ID_NULL)
    , mWeaponInfo(weaponInfo)
    , mShooter(shooter)
{
}

Projectile::~Projectile()
{
    if (mPhysicsBody)
    {
        gPhysics.DestroyPhysicsObject(mPhysicsBody);
    }
}

void Projectile::Spawn(const glm::vec3& position, cxx::angle_t heading)
{
    mSpawnPosition = position;
    mSpawnHeading = heading;
    if (mPhysicsBody == nullptr)
    {
        mPhysicsBody = gPhysics.CreatePhysicsObject(this, position, heading);
        debug_assert(mPhysicsBody);
    }
    else
    {   
        mPhysicsBody->SetPosition(position, heading);
    }

    // setup animation
    mAnimationState.Clear();

    if (mWeaponInfo)
    {
        StyleData& cityStyle = gGameMap.mStyleData;

        int objectindex = mWeaponInfo->mProjectileObject;
        if (objectindex > 0 && objectindex < (int) cityStyle.mObjects.size())
        {
            GameObjectInfo& objectInfo = cityStyle.mObjects[objectindex];
            debug_assert(objectInfo.mClassID == eGameObjectClass_Projectile);
            
            mAnimationState.Clear();
            mAnimationState.mAnimDesc = objectInfo.mAnimationData;

            eSpriteAnimLoop loopType = eSpriteAnimLoop_FromStart;
            if (!mWeaponInfo->IsFireDamage()) // todo: move to configs
            {
                loopType = eSpriteAnimLoop_None;
            }
            mAnimationState.PlayAnimation(loopType, eSpriteAnimMode_Normal, 24.0f); // todo: move to config
        }
    }

    // setup sprite rotation and scale
    cxx::angle_t rotationAngle = heading + cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    mDrawSprite.mRotateAngle = rotationAngle;
    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Projectiles;
}

void Projectile::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    mAnimationState.UpdateFrame(deltaTime);

    if (!mPhysicsBody->mContactDetected)
        return;

    if (mWeaponInfo == nullptr)
    {
        MarkForDeletion();
        return;
    }

    if (mPhysicsBody->mContactObject)
    {
        DamageInfo damageInfo;
        damageInfo.SetDamageFromWeapon(*mWeaponInfo, this);
        if (!mPhysicsBody->mContactObject->ReceiveDamage(damageInfo))
        {
            if (mWeaponInfo->IsFireDamage() || mPhysicsBody->mContactObject->IsPedestrianClass()) // todo: fix!
            {
                mPhysicsBody->ClearCurrentContact();
                return; // ignore contact
            }
        }
    }

    if (mWeaponInfo->IsExplosionDamage())
    {
        if (mPhysicsBody->mContactObject == nullptr)
        {
            mPhysicsBody->mContactPoint.y += Convert::MapUnitsToMeters(1.0f);
        }

        Explosion* explosion = gGameObjectsManager.CreateExplosion(mPhysicsBody->mContactObject, mShooter, eExplosionType_Rocket, mPhysicsBody->mContactPoint);
        debug_assert(explosion);
    }

    if (mWeaponInfo->mProjectileHitEffect > GameObjectType_Null)
    {
        GameObjectInfo& objectInfo = gGameMap.mStyleData.mObjects[mWeaponInfo->mProjectileHitEffect];
        Decoration* hitEffect = gGameObjectsManager.CreateDecoration(mPhysicsBody->mContactPoint, cxx::angle_t(), &objectInfo);
        debug_assert(hitEffect);

        if (hitEffect)
        {
            hitEffect->SetDrawOrder(eSpriteDrawOrder_Projectiles);
            hitEffect->SetLifeDuration(1);
        }
    }

    if (mWeaponInfo->mProjectileHitObjectSound != -1)
    {
        gAudioManager.PlaySfxLevel(mWeaponInfo->mProjectileHitObjectSound, GetPosition(), false);
    }

    MarkForDeletion();
}

void Projectile::PreDrawFrame()
{
    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);

    glm::vec3 position = mPhysicsBody->mSmoothPosition;
    ComputeDrawHeight(position);

    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
}

void Projectile::DebugDraw(DebugRenderer& debugRender)
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

    mDrawSprite.mHeight = maxHeight;
}

glm::vec3 Projectile::GetPosition() const
{
    return mPhysicsBody->GetPosition();
}

glm::vec2 Projectile::GetPosition2() const
{
    return mPhysicsBody->GetPosition2();
}