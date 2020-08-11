#pragma once

#include "GameDefs.h"

class SpriteBatch;
class DebugRenderer;

// defines base class of game entity
class GameObject: public cxx::noncopyable
{
    friend class GameObjectsManager;

public:
    const GameObjectID mObjectID; // its unique for all game objects except projectiles or effects, see GAMEOBJECT_ID_NULL
    const eGameObjectType mObjectTypeID;

public:
    virtual ~GameObject();

    // draw gameobject
    virtual void DrawFrame(SpriteBatch& spriteBatch)
    {
    }
    // process logic
    virtual void UpdateFrame()
    {
    }
    // draw debug info
    virtual void DrawDebug(DebugRenderer& debugRender)
    {
    }

protected:
    GameObject(eGameObjectType objectTypeID, GameObjectID uniqueID);
    
private:
    // internal stuff that can be touched only by PedestrianManager
    cxx::intrusive_node<GameObject> mObjectsNode; // updatable and drawable entities
    cxx::intrusive_node<GameObject> mDeleteObjectsNode; // to remove queue
};