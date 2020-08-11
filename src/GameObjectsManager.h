#pragma once

#include "Pedestrian.h"
#include "Vehicle.h"

// define game objects manager class
class GameObjectsManager final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    cxx::intrusive_list<GameObject> mObjectsList;
    cxx::intrusive_list<GameObject> mDeleteList;
    cxx::intrusive_list<Pedestrian> mPedestriansList;
    cxx::intrusive_list<Vehicle> mCarsList;

public:
    ~GameObjectsManager();

    bool Initialize();
    void Deinit();
    void UpdateFrame();
    void DebugDraw();

    // add pedestrian to map at specific location
    // @param position: Real world position
    // @param startRotation: Initial rotation
    Pedestrian* CreatePedestrian(const glm::vec3& startPosition, cxx::angle_t startRotation);

    // add car instance to map at specific location
    // @param startPosition: Initial world position
    // @param startRotation: Initial rotation
    // @param carStyle: Car style
    // @param carModel: Car model identifier
    Vehicle* CreateCar(const glm::vec3& startPosition, cxx::angle_t startRotation, CarStyle* carStyle);
    Vehicle* CreateCar(const glm::vec3& startPosition, cxx::angle_t startRotation, eCarModel carModel);

    // find gameobject by its unique identifier
    // @param objectID: Unique identifier
    Vehicle* GetCarByID(GameObjectID objectID) const;
    Pedestrian* GetPedestrianByID(GameObjectID objectID) const;
    GameObject* GetGameObjectByID(GameObjectID objectID) const;

    // will immediately destroy gameobject, don't call this mehod while UpdateFrame
    // @param object: Object to destroy
    void DestroyGameObject(GameObject* object);

    // queue gameobject for deletion, it will be destroyed next frame
    // @param object: Object to queue
    void MarkForDeletion(GameObject* object);

private:
    bool CreateStartupObjects();
    void DestroyObjectsInList(cxx::intrusive_list<GameObject>& objectsList);
    void DestroyPendingObjects();
    GameObjectID GenerateUniqueID();

private:
    GameObjectID mIDsCounter = 0;

    // objects pools
    cxx::object_pool<Pedestrian> mPedestriansPool;
    cxx::object_pool<Vehicle> mCarsPool;
};

extern GameObjectsManager gGameObjectsManager;