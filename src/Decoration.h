#pragma once

#include "GameObject.h"

class Decoration final: public GameObject
{
    friend class GameObjectsManager;

public:
    // readonly
    glm::vec3 mPosition;
    cxx::angle_t mRotation;

public:
    Decoration(GameObjectID id, GameObjectInfo* desc);
    ~Decoration();

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& position, cxx::angle_t heading);

    // Setup current position and rotation
    void SetTransform(const glm::vec3& position, cxx::angle_t heading);

    void SetLifeDuration(int numAnimationCycles);

    // Change current draw order for decoration object
    void SetDrawOrder(eSpriteDrawOrder drawOrder);

private:
    SpriteAnimation mAnimationState;

    int mLifeDuration = 0; // number of animation cycles before decoration will be deleted, or 0 for endless lifetime
};