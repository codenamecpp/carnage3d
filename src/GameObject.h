#pragma once

#include "GameDefs.h"
#include "DamageInfo.h"

class DebugRenderer;

// defines base class of game entity
class GameObject: public cxx::handled_object<GameObject>
{
    friend class GameObjectsManager;
    friend class MapRenderer;

public:
    const GameObjectID mObjectID; // its unique for all game objects except projectiles or effects, see GAMEOBJECT_ID_NULL
    const eGameObjectClass mClassID;

    // readonly
    eGameObjectFlags mFlags = eGameObjectFlags_None;

public:
    virtual ~GameObject();

    // update drawing sprite
    virtual void PreDrawFrame();

    // process logic
    virtual void UpdateFrame();

    // draw debug info
    virtual void DebugDraw(DebugRenderer& debugRender);

    // Process damage, it may be ignored depending on type of damage and objects current state
    // @param damageInfo: Damage details
    // @returns false if damage is ignored
    virtual bool ReceiveDamage(const DamageInfo& damageInfo);

    // schedule object to delete from game
    void MarkForDeletion();
    bool IsMarkedForDeletion() const;

    // Check if the game object can potentially be seen
    // @param screenBounds: Visible area
    bool IsOnScreen(const cxx::aabbox2d_t& screenBounds) const;

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
    inline bool IsPedestrianClass() const { return mClassID == eGameObjectClass_Pedestrian; }
    inline bool IsProjectileClass() const { return mClassID == eGameObjectClass_Projectile; }
    inline bool IsDecorationClass() const { return mClassID == eGameObjectClass_Decoration; }
    inline bool IsVehicleClass() const { return mClassID == eGameObjectClass_Car; }
    inline bool IsPowerupClass() const { return mClassID == eGameObjectClass_Powerup; }
    inline bool IsObstacleClass() const { return mClassID == eGameObjectClass_Obstacle; }
    inline bool IsExplosionClass() const { return mClassID == eGameObjectClass_Explosion; }

    // flag shortcuts
    inline bool IsInvisibleFlag() const { return (mFlags & eGameObjectFlags_Invisible) != 0; }
    inline bool IsCarObjectFlag() const { return (mFlags & eGameObjectFlags_CarObject) != 0; }
    inline bool IsTrafficFlag() const { return (mFlags & eGameObjectFlags_Traffic) != 0; }

protected:
    GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID);

    void RefreshDrawBounds();

protected:
    // todo: add attachment point and angle

    GameObject* mParentObject = nullptr;
    std::vector<GameObject*> mAttachedObjects;

    // drawing spricific data
    Sprite2D mDrawSprite;
    cxx::aabbox2d_t mDrawBounds; // sprite bounds cache

private:
    // marked object will be destroyed next game frame
    bool mMarkedForDeletion = false;
    unsigned int mLastRenderFrame = 0; // render frames counter
};