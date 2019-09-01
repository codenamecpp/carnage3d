#include "stdafx.h"
#include "Pedestrian.h"

Pedestrian::Pedestrian()
    : mPosition()
    , mPrevPosition()
    , mRotation()
    , mPrevRotation()
    , mSphereRadius(1.0f)
    , mDead()
{
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
}

//////////////////////////////////////////////////////////////////////////

bool PedestrianManager::Initialize()
{
    return true;
}

void PedestrianManager::Deinit()
{
}

void PedestrianManager::UpdateFrame(Timespan deltaTime)
{
    DestroyPendingPeds();
    
    for (Pedestrian* currentPed: mActivePedsList) // warning: dont add new peds during this loop
    {
        if (currentPed == nullptr)
            continue;

        currentPed->UpdateFrame(deltaTime);
    }

    RemoveOffscreenPeds();
    //AddRandomPed(); // todo

    // update physics
}

void PedestrianManager::DestroyPendingPeds()
{
    for (Pedestrian* currentPed: mDestroyPedsList)
    {
        if (currentPed == nullptr)
            continue;

        mPedsPool.destroy(currentPed);
    }
    mDestroyPedsList.clear();
}

void PedestrianManager::DestroyPedestrian(Pedestrian* pedestrian)
{
    debug_assert(pedestrian);
    if (pedestrian)
    {
        auto found_iter = std::find(mActivePedsList.begin(), mActivePedsList.end(), pedestrian);
        if (found_iter != mActivePedsList.end())
        {
            *found_iter = nullptr;
        }

        found_iter = std::find(mDestroyPedsList.begin(), mDestroyPedsList.end(), pedestrian);
        if (found_iter != mDestroyPedsList.end())
        {
            debug_assert(false);
            return;
        }

        mDestroyPedsList.push_back(pedestrian);
    }
}

void PedestrianManager::RemoveOffscreenPeds()
{
}

void PedestrianManager::AddRandomPed()
{
}