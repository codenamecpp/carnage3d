#pragma once

#include "GameObject.h"

class Explosion final: public GameObject
{
    friend class GameObjectsManager;

public:
    // readonly
    eExplosionType mExplosionType = eExplosionType_Rocket;
    GameObjectHandle mExplodingObject;
    PedestrianHandle mExplosionCauser;
  
public:
    // ctor
    // @param explodingObject: Object that exploded
    // @param causer: Pedestrian causing explosion
    // @param explosionType: Type identifier
    Explosion(GameObject* explodingObject, Pedestrian* causer, eExplosionType explosionType);

    // override GameObject
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;

private:
    void DamageObjectInContact();
    void DamagePedsNearby(bool enableInstantKill);
    void DamageCarsNearby();

private:
    SpriteAnimation mAnimationState;
    int mUpdatesCounter = 0;
};