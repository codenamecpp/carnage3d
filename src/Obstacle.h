#pragma once

#include "GameObject.h"

class Obstacle final: public GameObject
{
    friend class GameObjectsManager;

    // add runtime information support for gameobject
    decl_rtti(Obstacle, GameObject)

public:
    Obstacle(GameObjectID id, GameObjectStyle* desc);
    ~Obstacle();

    // override GameObject
    void DrawFrame(SpriteBatch& spriteBatch) override;
    void UpdateFrame() override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation);

private:
    SpriteAnimation mAnimationState;
    Sprite2D mDrawSprite;
    GameObjectStyle* mGameObjectDesc = nullptr;
};