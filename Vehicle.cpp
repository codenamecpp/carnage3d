#include "stdafx.h"
#include "Vehicle.h"

Vehicle::Vehicle()
{
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
    for (Vehicle* currCar: mActiveCarsList)
    {
        if (currCar)
        {
            mCarsPool.destroy(currCar);
        }
    }
    for (Vehicle* currCar: mDestroyCarsList)
    {
        if (currCar)
        {
            mCarsPool.destroy(currCar);
        }
    }
}

void CarsManager::UpdateFrame(Timespan deltaTime)
{
    DestroyPendingCars();
    
    for (Vehicle* currentCar: mActiveCarsList) // warning: dont add new cars during this loop
    {
        if (currentCar == nullptr)
            continue;

        currentCar->UpdateFrame(deltaTime);
    }
}

Vehicle* CarsManager::CreateCar(const glm::vec3& position)
{
    return nullptr;
}

void CarsManager::RemoveCar(Vehicle* car)
{
    debug_assert(car);
    if (car == nullptr)
        return;

    auto found_iter = std::find(mActiveCarsList.begin(), mActiveCarsList.end(), car);
    if (found_iter != mActiveCarsList.end())
    {
        *found_iter = nullptr;
    }

    found_iter = std::find(mDestroyCarsList.begin(), mDestroyCarsList.end(), car);
    if (found_iter != mDestroyCarsList.end())
    {
        debug_assert(false);
        return;
    }

    mDestroyCarsList.push_back(car);
}

void CarsManager::DestroyPendingCars()
{
    for (Vehicle* curr: mDestroyCarsList)
    {
        if (curr == nullptr)
            continue;

        mCarsPool.destroy(curr);
    }
    mDestroyCarsList.clear();
}

void CarsManager::AddToActiveList(Vehicle* car)
{
    for (Vehicle*& curr: mActiveCarsList)
    {
        if (curr == nullptr)
        {
            curr = car;
            return;
        }
    }
    mActiveCarsList.push_back(car);
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