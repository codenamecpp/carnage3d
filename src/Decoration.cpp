#include "stdafx.h"
#include "Decoration.h"

Decoration::Decoration(GameObjectID id) 
    : GameObject(eGameObjectClass_Decoration, id)
{
}

Decoration::~Decoration()
{
}

void Decoration::DrawFrame(SpriteBatch& spriteBatch)
{
}

void Decoration::UpdateFrame()
{
}

void Decoration::DrawDebug(DebugRenderer& debugRender)
{
}