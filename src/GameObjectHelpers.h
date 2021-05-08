#pragma once

#include "GameObject.h"
#include "Decoration.h"
#include "Explosion.h"
#include "Obstacle.h"
#include "Pedestrian.h"
#include "Projectile.h"
#include "Vehicle.h"

//////////////////////////////////////////////////////////////////////////
// Safe cast base to derived game object class
//////////////////////////////////////////////////////////////////////////

inline Decoration* ToDecoration(GameObject* gameObject)
{
    Decoration* objectPointer = nullptr;
    if (gameObject && gameObject->IsDecorationClass())
    {
        objectPointer = static_cast<Decoration*>(gameObject);
    }
    return objectPointer;
}

inline Pedestrian* ToPedestrian(GameObject* gameObject)
{
    Pedestrian* objectPointer = nullptr;
    if (gameObject && gameObject->IsPedestrianClass())
    {
        objectPointer = static_cast<Pedestrian*>(gameObject);
    }
    return objectPointer;
}

inline Explosion* ToExplosion(GameObject* gameObject)
{
    Explosion* objectPointer = nullptr;
    if (gameObject && gameObject->IsExplosionClass())
    {
        objectPointer = static_cast<Explosion*>(gameObject);
    }
    return objectPointer;
}

inline Obstacle* ToObstacle(GameObject* gameObject)
{
    Obstacle* objectPointer = nullptr;
    if (gameObject && gameObject->IsObstacleClass())
    {
        objectPointer = static_cast<Obstacle*>(gameObject);
    }
    return objectPointer;
}

inline Projectile* ToProjectile(GameObject* gameObject)
{
    Projectile* objectPointer = nullptr;
    if (gameObject && gameObject->IsProjectileClass())
    {
        objectPointer = static_cast<Projectile*>(gameObject);
    }
    return objectPointer;
}

inline Vehicle* ToVehicle(GameObject* gameObject)
{
    Vehicle* objectPointer = nullptr;
    if (gameObject && gameObject->IsVehicleClass())
    {
        objectPointer = static_cast<Vehicle*>(gameObject);
    }
    return objectPointer;
}

//////////////////////////////////////////////////////////////////////////

inline bool IsSameClass(GameObject* gameObjectA, GameObject* gameObjectB, eGameObjectClass gameObjectsClass)
{
    if (gameObjectA && gameObjectB)
    {
        return (gameObjectA->mClassID == gameObjectB->mClassID) && 
            (gameObjectA->mClassID == gameObjectsClass);
    }
    return false;
}

inline bool IsSameClass(GameObject* objectA, GameObject* objectB, GameObject* objectC, eGameObjectClass gameObjectsClass)
{
    if (objectA && objectB && objectC)
    {
        return (objectA->mClassID == objectB->mClassID) && 
            (objectA->mClassID == objectC->mClassID) && 
            (objectA->mClassID == gameObjectsClass);
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////