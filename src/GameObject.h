#pragma once

// defines base class of game objects
class GameObject: public cxx::noncopyable
{
public:
    GameObject(unsigned int uniqueID)
        : mID(uniqueID)
    {
    }

    virtual ~GameObject()
    {
    }

public:
    const unsigned int mID; // its unique for all game objects

    bool mMarkForDeletion = false; // destroy object at next frame
};