#pragma once

#include "GameObject.h"

class Obstacle final: public GameObject
{
    friend class GameObjectsManager;

public:
    Obstacle(GameObjectID id, GameObjectInfo* desc);
    ~Obstacle();

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void OnGameObjectSpawn() override;

    // Current world position
    glm::vec3 GetPosition() const override;
    glm::vec2 GetPosition2() const override;

private:
    SpriteAnimation mAnimationState;
    GameObjectInfo* mGameObjectDesc = nullptr;
};