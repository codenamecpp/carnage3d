#pragma once

#include "GameObject.h"

class Explosion final: public GameObject
{
    friend class GameObjectsManager;

    // add runtime information support for gameobject
    decl_rtti(Explosion, GameObject)

public:
    Explosion(GameObjectID id);
    ~Explosion();

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition);

private:
    SpriteAnimation mAnimationState;
};