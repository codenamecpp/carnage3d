#pragma once

// defines base class of game objects
class GameObject: public cxx::noncopyable
{
public:
    GameObject(GameObjectID_t uniqueID)
        : mObjectID(uniqueID)
    {
    }

    virtual ~GameObject()
    {
    }

public:
    const GameObjectID_t mObjectID; // its unique for all game objects

    bool mMarkForDeletion = false; // destroy object at next frame
};