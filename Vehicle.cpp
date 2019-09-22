#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsObject.h"

Vehicle::Vehicle(unsigned int id)
    : mActiveCarsNode(this)
    , mDeleteCarsNode(this)
    , mID(id)
    , mPhysicalBody()
    , mDead()
{
}

Vehicle::~Vehicle()
{
    if (mPhysicalBody)
    {
        gPhysics.DestroyPhysicsObject(mPhysicalBody);
    }
}

void Vehicle::EnterTheGame()
{
    glm::vec3 startPosition;
    
    //mPhysicalBody = gPhysics.CreatePedestrianBody(startPosition, 0.0f);
    //debug_assert(mPhysicalBody);

    //mMarkForDeletion = false;
    //mDead = false;
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
}

//////////////////////////////////////////////////////////////////////////

bool CarsManager::Initialize()
{
    mIDsCounter = 0;
    return true;
}

void CarsManager::Deinit()
{
    DestroyCarsInList(mActiveCarsList);
    DestroyCarsInList(mDeleteCarsList);
}

void CarsManager::UpdateFrame(Timespan deltaTime)
{
    DestroyPendingCars();
    
    bool hasDeleteCars = false;
    for (Vehicle* currentCar: mActiveCarsList) // warning: dont add or remove cars during this loop
    {
        if (!currentCar->mMarkForDeletion)
        {
            debug_assert(!mDeleteCarsList.contains(&currentCar->mDeleteCarsNode));
            currentCar->UpdateFrame(deltaTime);
        }

        if (currentCar->mMarkForDeletion)
        {
            mDeleteCarsList.insert(&currentCar->mDeleteCarsNode);
            hasDeleteCars = true;
        }
    }

    if (!hasDeleteCars)
        return;

    // deactivate all cars marked for deletion
    for (Vehicle* deleteCar: mDeleteCarsList)
    {
        RemoveFromActiveList(deleteCar);
    }
}

Vehicle* CarsManager::CreateCar(const glm::vec3& position)
{
    return nullptr;
}

void CarsManager::DestroyCar(Vehicle* car)
{
    debug_assert(car);
    if (car == nullptr)
        return;

    if (mDeleteCarsList.contains(&car->mDeleteCarsNode))
    {
        mDeleteCarsList.remove(&car->mDeleteCarsNode);
    }

    if (mActiveCarsList.contains(&car->mActiveCarsNode))
    {
        mActiveCarsList.remove(&car->mActiveCarsNode);
    }

    mCarsPool.destroy(car);
}

void CarsManager::DestroyPendingCars()
{
    DestroyCarsInList(mDeleteCarsList);
}

void CarsManager::AddToActiveList(Vehicle* car)
{
    debug_assert(car);
    if (car == nullptr)
        return;

    debug_assert(!mActiveCarsList.contains(&car->mActiveCarsNode));
    debug_assert(!mDeleteCarsList.contains(&car->mDeleteCarsNode));
    mActiveCarsList.insert(&car->mActiveCarsNode);
}

unsigned int CarsManager::GenerateUniqueID()
{
    unsigned int newID = ++mIDsCounter;
    if (newID == 0) // overflow
    {
        debug_assert(false);
    }
    return newID;
}

void CarsManager::RemoveFromActiveList(Vehicle* car)
{
    debug_assert(car);
    if (car && mActiveCarsList.contains(&car->mActiveCarsNode))
    {
        mActiveCarsList.remove(&car->mActiveCarsNode);
    }
}

void CarsManager::DestroyCarsInList(cxx::intrusive_list<Vehicle>& carsList)
{
    while (carsList.has_elements())
    {
        cxx::intrusive_node<Vehicle>* carNode = carsList.get_head_node();
        carsList.remove(carNode);

        Vehicle* carInstance = carNode->get_element();
        mCarsPool.destroy(carInstance);
    }
}
