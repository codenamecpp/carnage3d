#include "stdafx.h"
#include "GameObject.h"
#include "GameObjectsManager.h"

GameObject::GameObject(eGameObjectType objectTypeID, GameObjectID uniqueID)
    : mObjectID(uniqueID)
    , mObjectTypeID(objectTypeID)
    , mObjectsNode(this)
    , mDeleteObjectsNode(this)
{
    if (uniqueID == GAMEOBJECT_ID_NULL || 
        objectTypeID == eGameObjectType_Projectile)
    {
        debug_assert((uniqueID == GAMEOBJECT_ID_NULL) && (objectTypeID == eGameObjectType_Projectile));
    }
}

GameObject::~GameObject()
{
}

void GameObject::MarkForDeletion()
{
    gGameObjectsManager.MarkForDeletion(this);
}
