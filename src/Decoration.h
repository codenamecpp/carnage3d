#pragma once

#include "GameObject.h"

class Decoration final: public GameObject
{
    friend class GameObjectsManager;

    decl_rtti(Decoration, GameObject)

public:
    Decoration(GameObjectID id);
    ~Decoration();

    // override GameObject
    void DrawFrame(SpriteBatch& spriteBatch);
    void UpdateFrame();
    void DrawDebug(DebugRenderer& debugRender);

private:
    SpriteAnimation mAnimationState;
    Sprite2D mSprite;
};