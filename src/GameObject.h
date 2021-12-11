#pragma once

#include "GameDefs.h"
#include "DamageInfo.h"
#include "SfxDefs.h"
#include "Transform.h"
#include "PhysicsDefs.h"
#include "Collision.h"

// defines base class of game entity
class GameObject: public cxx::handled_object
{
    friend class GameObjectsManager;
    friend class MapRenderer;
    friend class PhysicsManager;

public:
    // readonly
    GameObjectID mObjectID; // its unique for all game objects except projectiles or effects, see GAMEOBJECT_ID_NULL
    GameObjectFlags mObjectFlags = GameObjectFlags_None;
    eGameObjectClass mClassID;

    // world space transform
    Transform mPreviousTransform; // prev frame transform, world space
    Transform mTransform; // current transform, world space
    Transform mTransformSmooth; // interpolated transform between prev and current frames for rendering, world space
    Transform mTransformLocal; // relative to parent transform, valid only if attached object

    PhysicsBody* mPhysicsBody = nullptr; // note that not all game objects has physics body

public:
    GameObject(eGameObjectClass objectTypeID, GameObjectID uniqueID);
    virtual ~GameObject();

    // Setup initial state when spawned or respawned on level
    virtual void HandleSpawn();

    // Cleanup all game object related resources - physics, logic, audio etc
    virtual void HandleDespawn();

    // Process logic
    virtual void UpdateFrame();

    // Process physics simulation
    virtual void SimulationStep();

    // Whether collision is possible between two contacting game objects
    virtual bool ShouldCollide(GameObject* otherObject) const;

    // Handle collision contact between game objects - after it was resolved
    virtual void HandleCollision(const Collision& collision);
    virtual void HandleCollisionWithMap(const MapCollision& collision);

    // Handle additional physics events
    virtual void HandleFallingStarts();
    virtual void HandleFallsOnGround(float fallDistance);
    virtual void HandleFallsOnWater(float fallDistance);

    // Draw debug info
    virtual void DebugDraw(DebugRenderer& debugRender);

    // Process damage, it may be ignored depending on type of damage and objects current state
    // @param damageInfo: Damage details
    // @returns false if damage is ignored
    virtual bool ReceiveDamage(const DamageInfo& damageInfo);

    // Set object's position and orientation within game world
    // @param transformSpace: Has meaning only for attached objects, otherwise 'world' and 'parent' are the same
    // @param newPosition, newOrientation: Position and rotation angle
    void SetTransform(const glm::vec3& newPosition, cxx::angle_t newOrientation, eTransformSpace transformSpace = eTransformSpace_Local);

    void SetPosition(const glm::vec3& newPosition, eTransformSpace transformSpace = eTransformSpace_Local);
    void SetPosition2(const glm::vec2& newPosition, eTransformSpace transformSpace = eTransformSpace_Local);

    void SetOrientation(cxx::angle_t newOrientation, eTransformSpace transformSpace = eTransformSpace_Local);
    void SetOrientation(const glm::vec2& directionVector);

    // Schedule object to delete from game
    void MarkForDeletion();
    bool IsMarkedForDeletion() const;

    // Check if the game object can potentially be seen
    // @param screenBounds: Visible area
    bool IsOnScreen(const cxx::aabbox2d_t& screenBounds) const;

    // Attach or detach object
    void AttachObject(GameObject* gameObject);
    void DetachObject(GameObject* gameObject);

    // Inspect hierarchy
    bool IsAttachedToObject() const;
    bool IsAttachedToObject(GameObject* gameObject) const;
    bool IsSameHierarchy(GameObject* gameObject);
    bool HasAttachedObjects();

    // Inspect hierarchy
    GameObject* GetParentObject() const;
    GameObject* GetAttachedObject(int index) const;

    // Audio shortcuts
    bool StartGameObjectSound(int ichannel, SfxSample* sfxSample, SfxFlags sfxFlags);
    bool StartGameObjectSound(int ichannel, eSfxSampleType sampleType, SfxSampleIndex sampleIndex, SfxFlags sfxFlags);
    void StopGameObjectSound(int ichannel);
    void StopGameObjectSounds();
    
    // Class shortcuts
    inline bool IsPedestrianClass() const { return mClassID == eGameObjectClass_Pedestrian; }
    inline bool IsProjectileClass() const { return mClassID == eGameObjectClass_Projectile; }
    inline bool IsDecorationClass() const { return mClassID == eGameObjectClass_Decoration; }
    inline bool IsVehicleClass() const { return mClassID == eGameObjectClass_Car; }
    inline bool IsPowerupClass() const { return mClassID == eGameObjectClass_Powerup; }
    inline bool IsObstacleClass() const { return mClassID == eGameObjectClass_Obstacle; }
    inline bool IsExplosionClass() const { return mClassID == eGameObjectClass_Explosion; }

    // Flag shortcuts
    inline bool IsInvisibleFlag() const { return (mObjectFlags & GameObjectFlags_Invisible) > 0; }
    inline bool IsCarPartFlag() const { return (mObjectFlags & GameObjectFlags_CarPart) > 0; }
    inline bool IsTrafficFlag() const { return (mObjectFlags & GameObjectFlags_Traffic) > 0; }
    inline bool IsMissionFlag() const { return (mObjectFlags & GameObjectFlags_Mission) > 0; }
    inline bool IsStartupFlag() const { return (mObjectFlags & GameObjectFlags_Startup) > 0; }

protected:
    void InitSounds();
    void FreeSounds();

    void SetSprite(int spriteIndex, SpriteDeltaBits deltaBits = 0);
    void SetPhysics(PhysicsBody* physicsBody);
    void SetParentObject(GameObject* gameObject);
    void RefreshDrawSprite();

    void OnParentTransformChanged();
    void OnTransformChanged();

    void SyncPhysicsTransform();
    void ClearContacts();
    void RegisterContact(const Contact& contactInfo);
    void UnregisterContactsWithObject(GameObject* otherObject);

    void InterpolateTransform(float factor);

protected:
    SfxEmitter* mSfxEmitter = nullptr;
    GameObject* mParentObject = nullptr;

    std::vector<GameObject*> mAttachedObjects;
    std::vector<Contact> mObjectsContacts; // list of contacting colliders (non triggers) on last simulation frame

    // drawing spricific data
    Sprite2D mDrawSprite;
    eSpriteOrientation mDrawSpriteOrientation = eSpriteOrientation_S;
    int mRemapClut = 0;
    cxx::aabbox2d_t mDrawBounds; // sprite bounds cache

private:
    // marked object will be destroyed next game frame
    bool mMarkedForDeletion = false;
    unsigned int mLastRenderFrame = 0; // render frames counter
};