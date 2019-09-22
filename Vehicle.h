#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"

// defines vehicle instance
class Vehicle final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    const unsigned int mID; // unique identifier

    PhysicsObject* mPhysicalBody;
    bool mDead;
    bool mMarkForDeletion;

    CarStyleData* mCarStyle; // cannot be null

public:
    // @param id: Unique object identifier, constant
    Vehicle(unsigned int id);
    ~Vehicle();

    // setup initial state when spawned on level
    void EnterTheGame();

    void UpdateFrame(Timespan deltaTime);

private:
    friend class CarsManager;

    // internal stuff that can be touched only by CarsManager
    cxx::intrusive_node<Vehicle> mActiveCarsNode;
    cxx::intrusive_node<Vehicle> mDeleteCarsNode;
};

//////////////////////////////////////////////////////////////////////////

// defines vehicles manager class
class CarsManager final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    cxx::intrusive_list<Vehicle> mActiveCarsList;
    cxx::intrusive_list<Vehicle> mDeleteCarsList;

public:
    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);

    // add car instance to map at specific location
    // @param position: Real world position
    // @param carTypeId: Index of car type in citystyle
    Vehicle* CreateCar(const glm::vec3& position, int carTypeId);

    // will immediately destroy car object, make sure it is not in use at this moment
    // @param car: Car instance
    void DestroyCar(Vehicle* car);

private:
    void DestroyCarsInList(cxx::intrusive_list<Vehicle>& carsList);
    void DestroyPendingCars();
    void AddToActiveList(Vehicle* car);
    void RemoveFromActiveList(Vehicle* car);

    unsigned int GenerateUniqueID();

private:
    unsigned int mIDsCounter;

    cxx::object_pool<Vehicle> mCarsPool;
};