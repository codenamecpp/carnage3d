#pragma once

#include "GameObject.h"

class Obstacle final: public GameObject
{
    friend class GameObjectsManager;

public:
    Obstacle(GameObjectID id, GameObjectInfo* desc);

    // override GameObject
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;

private:
    void SetupSpriteFrame();

private:
    SpriteAnimation mAnimationState;
    GameObjectInfo* mGameObjectDesc = nullptr;
};