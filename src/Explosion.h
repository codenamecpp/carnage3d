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
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;

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
    int mUpdatesCounter = 0;
};