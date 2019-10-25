#pragma once

// enums all possible gameobject types in game
enum eGameObjectType
{
    eGameObjectType_Pedestrian,
    eGameObjectType_Car,
    eGameObjectType_MapObject,
    eGameObjectType_COUNT,
};

decl_enum_strings(eGameObjectType);

// defines base class of game entity
class GameObject: public cxx::noncopyable
{
public:
    GameObject(eGameObjectType objectTypeID, GameObjectID_t uniqueID)
        : mObjectID(uniqueID)
        , mObjectTypeID(objectTypeID)
    {
    }

    virtual ~GameObject()
    {
    }

public:
    const GameObjectID_t mObjectID; // its unique for all game objects
    const eGameObjectType mObjectTypeID;

    bool mMarkForDeletion = false; // destroy object at next frame
};