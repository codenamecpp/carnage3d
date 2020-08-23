#pragma once

#include "GameObject.h"

class Explosion final: public GameObject
{
    friend class GameObjectsManager;
  
public:
    Explosion(GameObjectID id);
    ~Explosion();

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition);

    // Test whether explosion did its damage and can't hurt
    bool IsDamageDone() const;

private:
    void ProcessDamage();

private:
    SpriteAnimation mAnimationState;
    glm::vec3 mExplosionEpicentre;
    bool mDamageDone = false;
};