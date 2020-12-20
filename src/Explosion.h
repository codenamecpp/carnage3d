#pragma once

#include "GameObject.h"

class Explosion final: public GameObject
{
    friend class GameObjectsManager;
  
public:
    // ctor
    // @param explodingObject: Object that exploded
    // @param causer: Object causing explosion
    // @param explosionType: Type identifier
    Explosion(GameObject* explodingObject, GameObject* causer, eExplosionType explosionType);

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void Spawn(const glm::vec3& position, cxx::angle_t heading) override;

    // Current world position
    glm::vec3 GetPosition() const override;
    glm::vec2 GetPosition2() const override;

private:
    void DamageObjectInContact();
    void DamagePedsNearby(bool enableInstantKill);
    void DamageCarsNearby();

private:
    // params
    eExplosionType mExplosionType = eExplosionType_Rocket;
    GameObjectHandle mExplodingObject;
    GameObjectHandle mExplosionCauser;
    // state
    SpriteAnimation mAnimationState;
    float mDamageTimer = 0.0f;
    int mUpdatesCounter = 0;
};