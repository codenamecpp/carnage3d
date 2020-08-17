#pragma once

#include "GameDefs.h"

class SpriteBatch;
class DebugRenderer;

// defines base class of game entity
class GameObject: public cxx::noncopyable
{
    friend class GameObjectsManager;

public:
    const GameObjectID mObjectID; // its unique for all game objects except projectiles or effects, see GAMEOBJECT_ID_NULL
    const eGameObjectType mObjectTypeID;

public:
    virtual ~GameObject();

    // draw gameobject
    virtual void DrawFrame(SpriteBatch& spriteBatch)
    {
    }
    // process logic
    virtual void UpdateFrame()
    {
    }
    // draw debug info
    virtual void DrawDebug(DebugRenderer& debugRender)
    {
    }

    // schedule object to despawn
    void MarkForDeletion();

    // shortcuts
    inline bool IsPedestrianObject() const { return mObjectTypeID == eGameObjectType_Pedestrian; }
    inline bool IsProjectileObject() const { return mObjectTypeID == eGameObjectType_Projectile; }
    inline bool IsDecorationObject() const { return mObjectTypeID == eGameObjectType_Decoration; }
    inline bool IsCarObject() const { return mObjectTypeID == eGameObjectType_Car; }
    inline bool IsPowerupObject() const { return mObjectTypeID == eGameObjectType_Powerup; }
    inline bool IsObstacleObject() const { return mObjectTypeID == eGameObjectType_Obstacle; }

protected:
    GameObject(eGameObjectType objectTypeID, GameObjectID uniqueID);
    
private:
    cxx::intrusive_node<GameObject> mObjectsNode; // updatable and drawable entities
    cxx::intrusive_node<GameObject> mDeleteObjectsNode; // to remove queue
};