#pragma once

#include "GameDefs.h"

class DebugRenderer;

// defines base class of game entity
class GameObject: public cxx::noncopyable
{
    friend class GameObjectsManager;

    decl_rtti_base(GameObject)

public:
    const GameObjectID mObjectID; // its unique for all game objects except projectiles or effects, see GAMEOBJECT_ID_NULL
    const eGameObjectClass mObjectTypeID;

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

    // schedule object to delete from game
    void MarkForDeletion();

    bool IsMarkedForDeletion() const;

    // shortcuts
    inline bool IsPedestrianObject() const { return mObjectTypeID == eGameObjectClass_Pedestrian; }
    inline bool IsProjectileObject() const { return mObjectTypeID == eGameObjectClass_Projectile; }
    inline bool IsDecorationObject() const { return mObjectTypeID == eGameObjectClass_Decoration; }
    inline bool IsCarObject() const { return mObjectTypeID == eGameObjectClass_Car; }
    inline bool IsPowerupObject() const { return mObjectTypeID == eGameObjectClass_Powerup; }
    inline bool IsObstacleObject() const { return mObjectTypeID == eGameObjectClass_Obstacle; }

protected:
    GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID);

private:
    // marked object will be destroyed next game frame
    bool mMarkedForDeletion = false;
};