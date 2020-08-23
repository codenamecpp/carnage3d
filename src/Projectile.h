#pragma once

#include "WeaponInfo.h"
#include "GameObject.h"
#include "PhysicsComponents.h"

class Projectile final: public GameObject
{
    friend class GameObjectsManager;

public:
    // readonly
    WeaponInfo* mWeaponInfo = nullptr;
    ProjectilePhysicsBody* mPhysicsBody = nullptr;
    
    glm::vec3 mStartPosition;

public:
    Projectile();
    ~Projectile();

    // override GameObject
    void UpdateFrame() override;
    void PreDrawFrame() override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation, WeaponInfo* weaponInfo);

    // @param gameObject: null if contacting with map
    void SetContactDetected(const glm::vec3& position, GameObject* gameObject);
    bool IsContactDetected() const;

private:
    void ComputeDrawHeight(const glm::vec3& position);

private:
    SpriteAnimation mAnimationState;

    // collision contact info
    glm::vec3 mContactPoint;
    GameObject* mContactObject = nullptr;
    bool mContactDetected = false;
};