#include "stdafx.h"
#include "GameObject.h"
#include "GameObjectsManager.h"

GameObject::GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID)
    : mObjectID(uniqueID)
    , mObjectTypeID(objectTypeID)
{
    if (uniqueID == GAMEOBJECT_ID_NULL || 
        objectTypeID == eGameObjectClass_Projectile)
    {
        debug_assert((uniqueID == GAMEOBJECT_ID_NULL) && (objectTypeID == eGameObjectClass_Projectile));
    }
}

GameObject::~GameObject()
{
    SetDetached();
}

void GameObject::MarkForDeletion()
{
    gGameObjectsManager.MarkForDeletion(this);
}

bool GameObject::IsMarkedForDeletion() const
{
    return mMarkedForDeletion;
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
