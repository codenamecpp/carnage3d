#pragma once

#include "GameDefs.h"

class DebugRenderer;

// defines base class of game entity
class GameObject: public cxx::noncopyable
{
    friend class GameObjectsManager;
    friend class MapRenderer;

    // add runtime information support for gameobject
    decl_rtti_base(GameObject)

public:
    const GameObjectID mObjectID; // its unique for all game objects except projectiles or effects, see GAMEOBJECT_ID_NULL
    const eGameObjectClass mObjectTypeID;

    // readonly
    eGameObjectFlags mFlags = eGameObjectFlags_None;

public:
    virtual ~GameObject();

    // update drawing sprite
    virtual void PreDrawFrame()
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

    // Attach or detach object to other object
    void SetAttachedToObject(GameObject* parentObject);
    void SetDetached();

    // Inspect hierarchy
    bool IsAttachedToObject() const;
    bool IsAttachedToObject(GameObject* parentObject) const;
    bool HasAttachedObjects();

    // Inspect hierarchy
    GameObject* GetParentObject() const;
    GameObject* GetAttachedObject(int index) const;

    // class shortcuts
    inline bool IsPedestrianClass() const { return mObjectTypeID == eGameObjectClass_Pedestrian; }
    inline bool IsProjectileClass() const { return mObjectTypeID == eGameObjectClass_Projectile; }
    inline bool IsDecorationClass() const { return mObjectTypeID == eGameObjectClass_Decoration; }
    inline bool IsVehicleClass() const { return mObjectTypeID == eGameObjectClass_Car; }
    inline bool IsPowerupClass() const { return mObjectTypeID == eGameObjectClass_Powerup; }
    inline bool IsObstacleClass() const { return mObjectTypeID == eGameObjectClass_Obstacle; }

    // flag shortcuts
    inline bool IsInvisibleFlag() const { return (mFlags & eGameObjectFlags_Invisible) != 0; }
    inline bool IsCarObjectFlag() const { return (mFlags & eGameObjectFlags_CarObject) != 0; }

protected:
    GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID);

protected:
    // todo: add attachment point and angle

    GameObject* mParentObject = nullptr;
    std::vector<GameObject*> mAttachedObjects;

    // drawing spricific data
    Sprite2D mDrawSprite;


private:
    // marked object will be destroyed next game frame
    bool mMarkedForDeletion = false;

    unsigned int mLastRenderFrame = 0; // render frames counter
};