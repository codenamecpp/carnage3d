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

    // Disable primary of secondary damage of explosion
    void DisablePrimaryDamage();
    void DisableSecondaryDamage();

    // Test whether explosion did its damage and can't hurt
    bool IsDamageDone() const;

private:
    void ProcessPrimaryDamage();
    void ProcessSecondaryDamage();

private:
    SpriteAnimation mAnimationState;
    glm::vec3 mExplosionEpicentre;
   
    bool mPrimaryDamageDone = false;
    bool mSecondaryDamageDone = false;
};