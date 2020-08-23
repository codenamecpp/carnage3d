#pragma once

#include "GameObject.h"

class Decoration final: public GameObject
{
    friend class GameObjectsManager;

public:
    Decoration(GameObjectID id, GameObjectStyle* desc);
    ~Decoration();

    // override GameObject
    void PreDrawFrame() override;
    void UpdateFrame() override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // Setup initial state when spawned or respawned on level
    void Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation);

    void SetLifeDuration(int numAnimationCycles);

private:
    SpriteAnimation mAnimationState;
    GameObjectStyle* mGameObjectDesc = nullptr;

    int mLifeDuration = 0; // number of animation cycles before decoration will be deleted, or 0 for endless lifetime
};