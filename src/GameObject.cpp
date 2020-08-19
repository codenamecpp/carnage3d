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
}

void GameObject::MarkForDeletion()
{
    gGameObjectsManager.MarkForDeletion(this);
}

bool GameObject::IsMarkedForDeletion() const
{
    return mMarkedForDeletion;
}
