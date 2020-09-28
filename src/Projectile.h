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
    
public:
    Projectile(WeaponInfo* weaponInfo);
    ~Projectile();

    // override GameObject
    void UpdateFrame() override;
    void PreDrawFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void Spawn(const glm::vec3& position, cxx::angle_t heading) override;

    // Current world position
    glm::vec3 GetPosition() const override;
    glm::vec2 GetPosition2() const override;

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