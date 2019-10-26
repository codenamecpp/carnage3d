#pragma once

#include "GameDefs.h"

// defines base class of game entity
class GameObject: public cxx::noncopyable
{
public:
    GameObject(eGameObjectType objectTypeID, GameObjectID uniqueID)
        : mObjectID(uniqueID)
        , mObjectTypeID(objectTypeID)
    {
    }

    virtual ~GameObject()
    {
    }

public:
    const GameObjectID mObjectID; // its unique for all game objects
    const eGameObjectType mObjectTypeID;

    bool mMarkForDeletion = false; // destroy object at next frame
};