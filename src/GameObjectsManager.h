#pragma once

#include "Pedestrian.h"
#include "Vehicle.h"
#include "Projectile.h"
#include "Decoration.h"
#include "Obstacle.h"
#include "Explosion.h"

// define game objects manager class
class GameObjectsManager final: public cxx::noncopyable
{
public:
    // readonly
    std::vector<GameObject*> mAllObjectsList;
    std::vector<GameObject*> mDeleteObjectsList;

public:
    ~GameObjectsManager();

    bool InitGameObjects();
    void FreeGameObjects();

    void UpdateFrame();
    void DebugDraw();

    // Add new pedestrian instance to map at specific location
    // @param position: Real world position
    // @param heading: Initial rotation
    Pedestrian* CreatePedestrian(const glm::vec3& position, cxx::angle_t heading);

    // Add new car instance to map at specific location
    // @param position: Initial world position
    // @param heading: Initial rotation
    // @param desc: Car style
    // @param carModel: Car model identifier
    Vehicle* CreateCar(const glm::vec3& position, cxx::angle_t heading, CarStyle* desc);
    Vehicle* CreateCar(const glm::vec3& position, cxx::angle_t heading, eCarModel carModel);

    // Add new projectile instance to map at specific location
    Projectile* CreateProjectile(const glm::vec3& position, cxx::angle_t heading);
    Projectile* CreateProjectile(const glm::vec3& position, cxx::angle_t heading, WeaponInfo* weaponInfo);

    // Add new decoration instance to map at specific location
    Decoration* CreateDecoration(const glm::vec3& position, cxx::angle_t heading, GameObjectStyle* desc);

    // Add explosion instance to map at specific location 
    Explosion* CreateExplosion(const glm::vec3& position);

    // Add new obstacle instance to map at specific location
    Obstacle* CreateObstacle(const glm::vec3& position, cxx::angle_t heading, GameObjectStyle* desc);

    // Find gameobject by its unique identifier
    // @param objectID: Unique identifier
    Vehicle* GetVehicleByID(GameObjectID objectID) const;
    Obstacle* GetObstacleByID(GameObjectID objectID) const;
    Decoration* GetDecorationByID(GameObjectID objectID) const;
    Pedestrian* GetPedestrianByID(GameObjectID objectID) const;
    GameObject* GetGameObjectByID(GameObjectID objectID) const;

    // Get base hp points for specific car type
    // @param carType: Identifier
    int GetBaseHitpointsForCar(eCarVType carType) const;

    // Will immediately destroy gameobject, don't call this mehod during UpdateFrame
    // @param object: Object to destroy
    void DestroyGameObject(GameObject* object);

    // Queue gameobject for deletion, it will be destroyed next frame
    // @param object: Object to queue
    void MarkForDeletion(GameObject* object);

private:
    bool CreateStartupObjects();
    void DestroyAllObjects();
    void DestroyMarkedForDeletionObjects();
    GameObjectID GenerateUniqueID();

private:
    GameObjectID mIDsCounter = 0;

    // objects pools
    cxx::object_pool<Pedestrian> mPedestriansPool;
    cxx::object_pool<Vehicle> mCarsPool;
    cxx::object_pool<Projectile> mProjectilesPool;
    cxx::object_pool<Decoration> mDecorationsPool;
    cxx::object_pool<Obstacle> mObstaclesPool;
    cxx::object_pool<Explosion> mExplosionsPool;
};

extern GameObjectsManager gGameObjectsManager;