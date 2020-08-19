#pragma once

#include "GameObject.h"

class Decoration final: public GameObject
{
    friend class GameObjectsManager;

    decl_rtti(Decoration, GameObject)

public:
    Decoration(GameObjectID id, GameObjectStyle* desc);
    ~Decoration();

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