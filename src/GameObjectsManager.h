#pragma once

#include "Pedestrian.h"
#include "Vehicle.h"

// define game objects manager class
class GameObjectsManager final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    cxx::intrusive_list<Pedestrian> mActivePedestriansList;
    cxx::intrusive_list<Pedestrian> mDeletePedestriansList;

    cxx::intrusive_list<Vehicle> mActiveCarsList;
    cxx::intrusive_list<Vehicle> mDeleteCarsList;

public:
    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);
    void DebugDraw();

    // add pedestrian to map at specific location
    // @param position: Real world position
    Pedestrian* CreatePedestrian(const glm::vec3& position);

    // find pedestrian object by its unique identifier
    // @param objectID: Unique identifier
    Pedestrian* GetPedestrianByID(GameObjectID_t objectID) const;

    // add car instance to map at specific location
    // @param position: Real world position
    // @param carStyle: Car style
    // @param carModel: Car model identifier
    Vehicle* CreateCar(const glm::vec3& position, CarStyle* carStyle);
    Vehicle* CreateCar(const glm::vec3& position, eCarModel carModel);

    // find car object by its unique identifier
    // @param objectID: Unique identifier
    Vehicle* GetCarByID(GameObjectID_t objectID) const;

    // will immediately destroy game object, make sure it is not in use at this moment
    // @param object: Object to destroy
    void DestroyGameObject(Pedestrian* object);
    void DestroyGameObject(Vehicle* object);

private:
    void DestroyObjectsInList(cxx::intrusive_list<Pedestrian>& objectsList);
    void DestroyObjectsInList(cxx::intrusive_list<Vehicle>& objectsList);

    void AddToActiveList(Pedestrian* object);
    void AddToActiveList(Vehicle* object);

    void RemoveFromActiveList(Pedestrian* object);
    void RemoveFromActiveList(Vehicle* object);

    void DestroyPendingObjects();
    GameObjectID_t GenerateUniqueID();

private:
    GameObjectID_t mIDsCounter = 0;

    // objects pools
    cxx::object_pool<Pedestrian> mPedestriansPool;
    cxx::object_pool<Vehicle> mCarsPool;
};