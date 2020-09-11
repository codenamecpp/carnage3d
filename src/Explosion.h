#pragma once

#include "GameObject.h"

class Explosion final: public GameObject
{
    friend class GameObjectsManager;
  
public:
    Explosion();

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void Spawn(const glm::vec3& position, cxx::angle_t heading) override;

    // Current world position
    glm::vec3 GetCurrentPosition() const override;
    glm::vec2 GetCurrentPosition2() const override;

    // Disable primary of secondary damage of explosion
    void DisablePrimaryDamage();
    void DisableSecondaryDamage();

    // Set if it was car explosion
    void SetIsCarExplosion(Vehicle* carObject);

    // Test whether explosion did its damage and can't hurt
    bool IsDamageDone() const;
    bool IsCarExplosion() const;

private:
    void ProcessPrimaryDamage();
    void ProcessSecondaryDamage();

private:
    SpriteAnimation mAnimationState;

    bool mIsCarExplosion = false;
    bool mPrimaryDamageDone = false;
    bool mSecondaryDamageDone = false;
};