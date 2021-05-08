#pragma once

#include "GameObject.h"

class Decoration final: public GameObject
{
    friend class GameObjectsManager;

public:
    Decoration(GameObjectID id, GameObjectInfo* desc);

    // override GameObject
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;
    void HandleSpawn() override;

    // Setup current position and rotation
    void SetScale(float scale);
    void SetLifeDuration(int numAnimationCycles);

    // Set decoration move in direction
    // @param moveVelocity: Move velocity, meters per second
    void SetMoveVelocity(const glm::vec3& moveVelocity);

    // Change current draw order for decoration object
    void SetDrawOrder(eSpriteDrawOrder drawOrder);

private:
    SpriteAnimation mAnimationState;
    glm::vec3 mMoveVelocity;
    int mLifeDuration = 0; // number of animation cycles before decoration will be deleted, or 0 for endless lifetime
};