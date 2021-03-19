#include "stdafx.h"
#include "GameObject.h"
#include "GameObjectsManager.h"
#include "RenderingManager.h"
#include "AudioManager.h"

GameObject::GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID)
    : mObjectID(uniqueID)
    , mClassID(objectTypeID)
{
}

void GameObject::RefreshDrawBounds()
{
    mDrawSprite.GetApproximateBounds(mDrawBounds);
}

GameObject::~GameObject()
{
    SetDetached();

    // audio
    if (mSfxEmitter)
    {
        mSfxEmitter->ReleaseEmitter(false); // leave sounds
        mSfxEmitter = nullptr;
    }
}

glm::vec3 GameObject::GetPosition() const
{
    // do nothing
    return {};
}

glm::vec2 GameObject::GetPosition2() const
{
    // do nothing
    return {};
}

void GameObject::PreDrawFrame()
{
    // do nothing
}

void GameObject::UpdateFrame()
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

void GameObject::Spawn(const glm::vec3& spawnPosition, cxx::angle_t spawnHeading)
{
    mSpawnPosition = spawnPosition;
    mSpawnHeading = spawnHeading;

    // init audio
    if (mSfxEmitter == nullptr)
    {
        mSfxEmitter = gAudioManager.CreateEmitter(this, spawnPosition);
        debug_assert(mSfxEmitter);
    }
    else
    {
        mSfxEmitter->StopAllSounds(); // reset
    }

    OnGameObjectSpawn();
}

void GameObject::OnGameObjectSpawn()
{
    // do nothing
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

void GameObject::SetAttachedToObject(GameObject* parentObject)
{
    if (parentObject == nullptr || parentObject == this)
    {
        debug_assert(false);
        return;
    }

    if (mParentObject == parentObject)
        return;

    if (mParentObject)
    {
        SetDetached();
    }

    mParentObject = parentObject;
    mParentObject->mAttachedObjects.push_back(this);
}

void GameObject::SetDetached()
{
    if (mParentObject == nullptr)
        return;

    cxx::erase_elements(mParentObject->mAttachedObjects, this);
    mParentObject = nullptr;
}

bool GameObject::IsAttachedToObject() const
{
    return mParentObject != nullptr;
}

bool GameObject::IsAttachedToObject(GameObject* parentObject) const
{
    return mParentObject && (mParentObject == parentObject);
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
    if (mSfxEmitter)
    {
        mSfxEmitter->UpdateEmitterParams(GetPosition()); // force sync params
        return mSfxEmitter->StartSound(ichannel, sfxSample, sfxFlags);
    }
    return false;
}

bool GameObject::StartGameObjectSound(int ichannel, eSfxType sfxType, SfxIndex sfxIndex, SfxFlags sfxFlags)
{
    if (mSfxEmitter)
    {
        SfxSample* sfxSample = gAudioManager.GetSound(sfxType, sfxIndex);
        if (sfxSample)
        {
            mSfxEmitter->UpdateEmitterParams(GetPosition()); // force sync params
            return mSfxEmitter->StartSound(ichannel, sfxSample, sfxFlags);
        }
    }
    return false;
}