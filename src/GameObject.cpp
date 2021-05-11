#include "stdafx.h"
#include "GameObject.h"
#include "GameObjectsManager.h"
#include "RenderingManager.h"
#include "AudioManager.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "SpriteManager.h"

GameObject::GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID)
    : mObjectID(uniqueID)
    , mClassID(objectTypeID)
{
}

GameObject::~GameObject()
{
    // note: make all necessary cleanups in HandleDespawn, not in destructor!

    debug_assert(mPhysicsBody == nullptr);
    debug_assert(mSfxEmitter == nullptr);
    debug_assert(mParentObject == nullptr);
}

void GameObject::InitSounds()
{
    if (mSfxEmitter == nullptr)
    {
        mSfxEmitter = gAudioManager.CreateEmitter(this, mTransform.mPosition);
        debug_assert(mSfxEmitter);
    }
}

void GameObject::FreeSounds()
{
    // release audio emitter
    if (mSfxEmitter)
    {
        mSfxEmitter->ReleaseEmitter(false); // leave sounds
        mSfxEmitter = nullptr;
    }
}

void GameObject::SetSprite(int spriteIndex, SpriteDeltaBits deltaBits)
{
    if (deltaBits > 0)
    {
        gSpriteManager.GetSpriteTexture(mObjectID, spriteIndex, mRemapClut, deltaBits, mDrawSprite);
    }
    else
    {
        gSpriteManager.GetSpriteTexture(mObjectID, spriteIndex, mRemapClut, mDrawSprite);
    }
    RefreshDrawSprite();
}

void GameObject::SetPhysics(PhysicsBody* physicsBody)
{
    if (mPhysicsBody)
    {
        gPhysics.DestroyBody(mPhysicsBody);
        mPhysicsBody = nullptr;
    }

    mPhysicsBody = physicsBody;

    if (mPhysicsBody)
    {
        if (mParentObject)
        {
            mPhysicsBody->ChangeFlags(PhysicsBodyFlags_Linked, PhysicsBodyFlags_None);
        }
        else
        {
            mPhysicsBody->ChangeFlags(PhysicsBodyFlags_None, PhysicsBodyFlags_Linked);
        }
        mPhysicsBody->ClearForces();
        mPhysicsBody->SetTransform(mTransform.mPosition, mTransform.mOrientation);
    }

    ClearContacts();
}

void GameObject::UpdateFrame()
{
    // do nothing
}

void GameObject::SimulationStep()
{
    // do nothing
}

bool GameObject::ShouldCollide(GameObject* otherObject) const
{
    return true;
}

void GameObject::HandleCollision(const Collision& collision)
{
    // do nothing
}

void GameObject::HandleCollisionWithMap(const MapCollision& collision)
{
    // do nothing
}

void GameObject::HandleFallingStarts()
{
    // do nothing
}

void GameObject::HandleFallsOnGround(float fallDistance)
{
    // do nothing
}

void GameObject::HandleFallsOnWater(float fallDistance)
{
    // do nothing
}

void GameObject::DebugDraw(DebugRenderer& debugRender)
{
    // do nothing
}

bool GameObject::ReceiveDamage(const DamageInfo& damageInfo)
{
    // do nothing
    return false;
}

void GameObject::RefreshDrawSprite()
{
    cxx::angle_t angleOffset;
    if (mDrawSpriteOrientation == eSpriteOrientation_N)
    {
        angleOffset = cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    }
    if (mDrawSpriteOrientation == eSpriteOrientation_S)
    {
        angleOffset = -cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    }

    mDrawSprite.mRotateAngle = mTransformSmooth.mOrientation + angleOffset;

    mDrawSprite.mPosition.x = mTransformSmooth.mPosition.x;
    mDrawSprite.mPosition.y = mTransformSmooth.mPosition.z;

    mDrawSprite.mHeight = mTransformSmooth.mPosition.y;

    if (mParentObject)
    {
        if (mParentObject->mDrawSprite.mHeight > mDrawSprite.mHeight)
        {
            mDrawSprite.mHeight = mParentObject->mDrawSprite.mHeight;
        }
    }
    else if ((mPhysicsBody == nullptr) || !mPhysicsBody->mFalling)
    // compute draw height for non-attached objects
    {
        float newDrawHeight = mDrawSprite.mHeight;

        if (IsPedestrianClass())
        {
            const float halfBox = Convert::PixelsToMeters(PED_SPRITE_DRAW_BOX_SIZE_PX) * 0.5f;
            glm::vec3 points[4] = 
            {
                { -halfBox, mTransformSmooth.mPosition.y + 0.01f, -halfBox },
                {  halfBox, mTransformSmooth.mPosition.y + 0.01f, -halfBox },
                {  halfBox, mTransformSmooth.mPosition.y + 0.01f,  halfBox },
                { -halfBox, mTransformSmooth.mPosition.y + 0.01f,  halfBox },
            };
            for (glm::vec3& currPoint: points)
            {
                currPoint.x += mTransformSmooth.mPosition.x;
                currPoint.z += mTransformSmooth.mPosition.z;
                // get height
                float height = gGameMap.GetHeightAtPosition(currPoint);
                if (height > newDrawHeight)
                {
                    newDrawHeight = height;
                }
            }
        }
        else
        {
            glm::vec2 corners[4];
            mDrawSprite.GetCorners(corners);
            for (glm::vec2& currCorner: corners)
            {
                float height = gGameMap.GetHeightAtPosition(glm::vec3(currCorner.x, mTransformSmooth.mPosition.y, currCorner.y));
                if (height > newDrawHeight)
                {
                    newDrawHeight = height;
                }
            }
        }

        mDrawSprite.mHeight = newDrawHeight;
    }

    mDrawSprite.GetApproximateBounds(mDrawBounds);
}

void GameObject::OnParentTransformChanged()
{
    debug_assert(mParentObject);
    
    if (mParentObject)
    {
        mTransform.mPosition = mParentObject->mTransform.GetPoint(mTransformLocal.mPosition, eTransformSpace_World);
        mTransform.mOrientation = (mParentObject->mTransform.mOrientation + mTransformLocal.mOrientation);
    }

    OnTransformChanged();
}

void GameObject::OnTransformChanged()
{
    mPreviousTransform = mTransform;
    mTransformSmooth = mTransform;

    // sync physics
    if (mPhysicsBody)
    {
        mPhysicsBody->SetTransform(mTransform.mPosition, mTransform.mOrientation);
    }

    RefreshDrawSprite();

    // update attached objects
    for (GameObject* currObject: mAttachedObjects)
    {
        currObject->OnParentTransformChanged();
    }
}

void GameObject::SyncPhysicsTransform()
{
    if (mParentObject == nullptr) // hierarchy root
    {
        if ((mPhysicsBody == nullptr) || (mPhysicsBody->IsAwake() == false) ||
            (mPhysicsBody->CheckFlags(PhysicsBodyFlags_Static)))
        {
            // no need to synchronize
            return;
        }

        if (mPreviousTransform != mTransform)
        {
            mPreviousTransform = mTransform;
            mTransformSmooth = mTransform;
        }

        Transform newTransform( mPhysicsBody->GetPosition(), mPhysicsBody->GetOrientation() );
        if (newTransform == mTransform)
            return; // transform not changed

        mTransform = newTransform;
    }
    else // child
    {
        if (mPreviousTransform != mTransform)
        {
            mPreviousTransform = mTransform;
            mTransformSmooth = mTransform;
        }

        Transform newTransform;
        newTransform.mPosition = mParentObject->mTransform.GetPoint(mTransformLocal.mPosition, eTransformSpace_World);
        newTransform.mOrientation = (mParentObject->mTransform.mOrientation + mTransformLocal.mOrientation);
        if (newTransform == mTransform)
            return; // transform not changed

        mTransform = newTransform;

        // set physics transform
        if (mPhysicsBody)
        {
            mPhysicsBody->SetTransform(mTransform.mPosition, mTransform.mOrientation);
        }
    }

    RefreshDrawSprite();

    // propagate sync to attached objects
    for (GameObject* currObject: mAttachedObjects)
    {
        currObject->SyncPhysicsTransform();
    }
}

void GameObject::ClearContacts()
{
    mObjectsContacts.clear();
}

void GameObject::RegisterContact(const Contact& contactInfo)
{
    debug_assert(contactInfo.mThisObject == this);
    debug_assert(contactInfo.mThatObject && (contactInfo.mThatObject != contactInfo.mThisObject));

    mObjectsContacts.push_back(contactInfo);
}

void GameObject::UnregisterContactsWithObject(GameObject* otherObject)
{
    debug_assert(otherObject);

    cxx::erase_elements_if(mObjectsContacts, [otherObject](const Contact& currContact)
    {
        return (currContact.mThatObject == otherObject);
    });
}

void GameObject::InterpolateTransform(float factor)
{
    if (mPreviousTransform == mTransform)
    {
        debug_assert(mTransformSmooth == mTransform);
        return;
    }

    debug_assert((factor >= 0.0f) && (factor <= 1.0f));
    mTransformSmooth = ::InterpolateTransform(mPreviousTransform, mTransform, factor);
    RefreshDrawSprite();
    for (GameObject* currChild: mAttachedObjects)
    {
        currChild->InterpolateTransform(factor);
    }
}

void GameObject::SetTransform(const glm::vec3& newPosition, cxx::angle_t newOrientation, eTransformSpace transformSpace)
{
    bool transformChanged = false;
    if ((transformSpace == eTransformSpace_Local) && mParentObject)
    {
        bool computeWorldTransform = false;

        if (mTransformLocal.mOrientation != newOrientation)
        {
            transformChanged = true;
            computeWorldTransform = true;
            mTransformLocal.mOrientation = newOrientation;
        }

        if (mTransformLocal.mPosition != newPosition)
        {
            transformChanged = true;
            computeWorldTransform = true;
            mTransformLocal.mPosition = newPosition;
        }

        if (computeWorldTransform)
        {
            mTransform.mOrientation = mParentObject->mTransform.mOrientation + newOrientation; 
            mTransform.mPosition = mParentObject->mTransform.GetPoint(newPosition, eTransformSpace_World);
        }
    }
    else // global space
    {
        if (mParentObject)
        {
            // convert to local space

            cxx::angle_t newOrientationLocal = newOrientation - mParentObject->mTransform.mOrientation;
            if (mTransformLocal.mOrientation != newOrientationLocal)
            {
                transformChanged = true;
                mTransformLocal.mOrientation = newOrientationLocal;
                // update world transform
                mTransform.mOrientation = newOrientation;
            }

            glm::vec3 newPositionLocal = mParentObject->mTransform.GetPoint(newPosition, eTransformSpace_Local);
            if (mTransformLocal.mPosition != newPositionLocal)
            {
                transformChanged = true;
                mTransformLocal.mPosition = newPositionLocal;
                // update world transform
                mTransform.mPosition = newPosition;
            }
        }
        else
        {
            if (mTransform.mPosition != newPosition)
            {
                transformChanged = true;
                mTransform.mPosition = newPosition;
            }

            if (mTransform.mOrientation != newOrientation)
            {
                transformChanged = true;
                mTransform.mOrientation = newOrientation;
            }
        }
    }

    if (transformChanged)
    {
        OnTransformChanged();
    }
}

void GameObject::SetPosition(const glm::vec3& newPosition, eTransformSpace transformSpace)
{
    const Transform* currTransform = &mTransform;
    if ((transformSpace == eTransformSpace_Local) && mParentObject)
    {
        currTransform = &mTransformLocal;
    }

    if (currTransform->mPosition == newPosition)
        return;

    SetTransform(newPosition, currTransform->mOrientation, transformSpace);
}

void GameObject::SetPosition2(const glm::vec2& newPosition2, eTransformSpace transformSpace)
{
    const Transform* currTransform = &mTransform;
    if ((transformSpace == eTransformSpace_Local) && mParentObject)
    {
        currTransform = &mTransformLocal;
    }

    if ((currTransform->mPosition.x == newPosition2.x) && 
        (currTransform->mPosition.z == newPosition2.y))
    {
        return;
    }

    glm::vec3 newPosition (newPosition2.x, currTransform->mPosition.y, newPosition2.y);
    SetTransform(newPosition, currTransform->mOrientation, transformSpace);
}

void GameObject::SetOrientation(cxx::angle_t newOrientation, eTransformSpace transformSpace)
{
    const Transform* currTransform = &mTransform;
    if ((transformSpace == eTransformSpace_Local) && mParentObject)
    {
        currTransform = &mTransformLocal;
    }

    if (currTransform->mOrientation == newOrientation)
        return;

    SetTransform(currTransform->mPosition, newOrientation, transformSpace);
}

void GameObject::SetOrientation(const glm::vec2& directionVector)
{
    Transform newTransform;    
    newTransform.SetOrientation(directionVector);
    SetTransform(mTransform.mPosition, newTransform.mOrientation, eTransformSpace_World);
}

void GameObject::HandleSpawn()
{
    // do nothing
}

void GameObject::HandleDespawn()
{
    // set detached
    SetParentObject(nullptr);

    // destroy physics body
    SetPhysics(nullptr);

    // detach childs
    while (!mAttachedObjects.empty())
    {
        GameObject* gameObject = mAttachedObjects.back();
        DetachObject(gameObject);
    }
    FreeSounds();
}

void GameObject::MarkForDeletion()
{
    mMarkedForDeletion = true;
}

bool GameObject::IsMarkedForDeletion() const
{
    return mMarkedForDeletion;
}

bool GameObject::IsOnScreen(const cxx::aabbox2d_t& screenBounds) const
{
    return screenBounds.contains(mDrawBounds);
}

void GameObject::SetParentObject(GameObject* gameObject)
{
    debug_assert(gameObject != this);

    if (mParentObject)
    {
        cxx::erase_elements(mParentObject->mAttachedObjects, this);
    }
    mParentObject = gameObject;
    if (mParentObject)
    {
        mParentObject->mAttachedObjects.push_back(this);
    }
    
    mTransformLocal.SetIdentity();

    // link physical body to parent object
    if (mPhysicsBody)
    {
        mPhysicsBody->ClearForces();
        if (mParentObject)
        {
            mPhysicsBody->ChangeFlags(PhysicsBodyFlags_Linked, PhysicsBodyFlags_None);
        }
        else
        {
            mPhysicsBody->ChangeFlags(PhysicsBodyFlags_None, PhysicsBodyFlags_Linked);
        }
    }
}

void GameObject::AttachObject(GameObject* gameObject)
{
    if ((gameObject == nullptr) || (gameObject == this))
    {
        debug_assert(false);
        return;
    }

    if (gameObject->mParentObject == this) // child already attached
        return;

    gameObject->SetParentObject(this);
    gameObject->OnParentTransformChanged();
}

void GameObject::DetachObject(GameObject* gameObject)
{
    if ((gameObject == nullptr) || (gameObject->mParentObject != this))
    {
        debug_assert(false);
        return;
    }

    gameObject->SetParentObject(nullptr);
}

bool GameObject::IsAttachedToObject() const
{
    return mParentObject != nullptr;
}

bool GameObject::IsAttachedToObject(GameObject* gameObject) const
{
    return mParentObject && (mParentObject == gameObject);
}

bool GameObject::IsSameHierarchy(GameObject* gameObject)
{
    if ((gameObject == this) || (gameObject == nullptr))
    {
        return (gameObject == this);
    }

    // check own parents
    for (GameObject* parentObject = mParentObject; (parentObject != nullptr); 
        parentObject = parentObject->mParentObject)
    {
        if (parentObject == gameObject)
            return true;
    }
    // check other parents
    for (GameObject* parentObject = gameObject->mParentObject; (parentObject != nullptr); 
        parentObject = parentObject->mParentObject)
    {
        if (parentObject == this)
            return true;
    }
    return false;
}

bool GameObject::HasAttachedObjects()
{
    return !mAttachedObjects.empty();
}

GameObject* GameObject::GetParentObject() const
{
    return mParentObject;
}

GameObject* GameObject::GetAttachedObject(int index) const
{
    GameObject* gameobject = nullptr;
    debug_assert(index >= 0);
    if (index < (int) mAttachedObjects.size())
    {
        gameobject = mAttachedObjects[index];
    }
    return gameobject;
}

bool GameObject::StartGameObjectSound(int ichannel, SfxSample* sfxSample, SfxFlags sfxFlags)
{
    if (mSfxEmitter == nullptr)
    {
        InitSounds();
    }

    if (mSfxEmitter)
    {
        mSfxEmitter->UpdateEmitterParams(mTransformSmooth.mPosition); // force sync params
        return mSfxEmitter->StartSound(ichannel, sfxSample, sfxFlags);
    }
    return false;
}

bool GameObject::StartGameObjectSound(int ichannel, eSfxSampleType sampleType, SfxSampleIndex sampleIndex, SfxFlags sfxFlags)
{
    if (mSfxEmitter == nullptr)
    {
        InitSounds();
    }

    if (mSfxEmitter)
    {
        SfxSample* sfxSample = gAudioManager.GetSound(sampleType, sampleIndex);
        if (sfxSample)
        {
            mSfxEmitter->UpdateEmitterParams(mTransformSmooth.mPosition); // force sync params
            return mSfxEmitter->StartSound(ichannel, sfxSample, sfxFlags);
        }
    }
    return false;
}

void GameObject::StopGameObjectSounds()
{
    if (mSfxEmitter)
    {
        mSfxEmitter->StopAllSounds();
    }
}
