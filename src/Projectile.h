#pragma once

#include "WeaponInfo.h"
#include "GameObject.h"
#include "PhysicsBody.h"

class Projectile final: public GameObject
{
    friend class GameObjectsManager;

public:
    // readonly
    WeaponInfo* mWeaponInfo = nullptr;
    PedestrianHandle mShooter;
    glm::vec3 mStartPosition;
    
public:
    Projectile(WeaponInfo* weaponInfo, Pedestrian* shooter);

    // override GameObject
    void UpdateFrame() override;
    void SimulationStep() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;
    bool ShouldCollide(GameObject* otherObject) const override;
    void HandleCollisionWithMap(const MapCollision& collision) override;

private:
    void ClearCurrentHit();

private:
    SpriteAnimation mAnimationState;

    bool mHitSomething = false;
    GameObjectHandle mHitObject; // null if hit wall
    ContactPoint mHitPoint;
};