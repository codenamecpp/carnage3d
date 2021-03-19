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
    ProjectilePhysics* mPhysicsBody = nullptr;
    PedestrianHandle mShooter;
    
public:
    Projectile(WeaponInfo* weaponInfo, Pedestrian* shooter);
    ~Projectile();

    // override GameObject
    void UpdateFrame() override;
    void PreDrawFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void OnGameObjectSpawn() override;

    // Current world position
    glm::vec3 GetPosition() const override;
    glm::vec2 GetPosition2() const override;

private:
    void ComputeDrawHeight(const glm::vec3& position);

private:
    SpriteAnimation mAnimationState;
};