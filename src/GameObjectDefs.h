#pragma once

// forwards

class GameObject;
class Pedestrian;
class Vehicle;
class Projectile;
class Decoration;
class Obstacle;
class Explosion;

// some game objects has null identifier, they are dacals, projectiles and short-lived effects
#define GAMEOBJECT_ID_NULL 0

using GameObjectID = unsigned int; // unique id of gameobject instance in world

// gameobject class identifiers
enum eGameObjectClass
{
    eGameObjectClass_Car,
    eGameObjectClass_Pedestrian,
    eGameObjectClass_Projectile,
    eGameObjectClass_Powerup,
    eGameObjectClass_Decoration,
    eGameObjectClass_Obstacle,
    eGameObjectClass_Explosion,
    eGameObjectClass_COUNT,
};

decl_enum_strings(eGameObjectClass);

enum eGameObjectFlags: unsigned short
{
    eGameObjectFlags_None = 0,
    eGameObjectFlags_Invisible = BIT(0),
    eGameObjectFlags_CarObject = BIT(1),
};

decl_enum_as_flags(eGameObjectFlags);

// game object type indices
enum
{
    //GameObjectIndex_


    GameObjectType_MAX = 102
};