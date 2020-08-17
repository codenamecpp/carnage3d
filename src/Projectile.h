#pragma once

#include "GameObject.h"
#include "PhysicsComponents.h"

class Projectile final: public GameObject
{
    friend class GameObjectsManager;

public:
    // readonly
    ProjectileStyle* mProjectileStyle = nullptr;
    ProjectilePhysicsBody* mPhysicsBody = nullptr;
    float mDrawHeight;
    
    bool mDead; // whether projectile should be removed from map
    glm::vec3 mStartPosition;

public:
    Projectile(ProjectileStyle* style);
    ~Projectile();

    // override GameObject methods
    void UpdateFrame() override;
    void DrawFrame(SpriteBatch& spriteBatch) override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation);

private:
    void ComputeDrawHeight(const glm::vec3& position);

private:
    SpriteAnimation mAnimationState;
    Sprite2D mDrawSprite;

    cxx::intrusive_node<Projectile> mProjectilesListNode;
};