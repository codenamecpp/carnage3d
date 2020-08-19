#pragma once

#include "Pedestrian.h"
#include "Vehicle.h"
#include "Projectile.h"
#include "Decoration.h"

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

    // New projectile instance to map at specific location
    Projectile* CreateProjectile(const glm::vec3& position, cxx::angle_t heading, eProjectileType typeID);
    Projectile* CreateProjectile(const glm::vec3& position, cxx::angle_t heading, ProjectileStyle* desc);

    // find gameobject by its unique identifier
    // @param objectID: Unique identifier
    Vehicle* GetVehicleByID(GameObjectID objectID) const;
    Decoration* GetDecorationByID(GameObjectID objectID) const;
    Pedestrian* GetPedestrianByID(GameObjectID objectID) const;
    GameObject* GetGameObjectByID(GameObjectID objectID) const;

    // will immediately destroy gameobject, don't call this mehod during UpdateFrame
    // @param object: Object to destroy
    void DestroyGameObject(GameObject* object);

    // queue gameobject for deletion, it will be destroyed next frame
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
};

extern GameObjectsManager gGameObjectsManager;