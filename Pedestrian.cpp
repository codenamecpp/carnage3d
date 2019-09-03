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
    if (mAnimation.IsAnimationActive())
    {
        mAnimation.UpdateFrame(deltaTime);
    }
}

//////////////////////////////////////////////////////////////////////////

bool PedestrianManager::Initialize()
{
    return true;
}

void PedestrianManager::Deinit()
{
    for (Pedestrian* currPedestrian: mActivePedsList)
    {
        if (currPedestrian)
        {
            mPedsPool.destroy(currPedestrian);
        }
    }
    for (Pedestrian* currPedestrian: mDestroyPedsList)
    {
        if (currPedestrian)
        {
            mPedsPool.destroy(currPedestrian);
        }
    }
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

void PedestrianManager::RemovePedestrian(Pedestrian* pedestrian)
{
    debug_assert(pedestrian);
    if (pedestrian == nullptr)
        return;

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

void PedestrianManager::RemoveOffscreenPeds()
{
}

Pedestrian* PedestrianManager::CreateRandomPed(const glm::vec3& position)
{
    Pedestrian* instance = mPedsPool.create();
    debug_assert(instance);

    AddToActiveList(instance);

    // init
    instance->mPosition = position;
    instance->mPrevPosition = position;
    return instance;
}

void PedestrianManager::AddToActiveList(Pedestrian* ped)
{
    for (Pedestrian*& curr: mActivePedsList)
    {
        if (curr == nullptr)
        {
            curr = ped;
            return;
        }
    }
    mActivePedsList.push_back(ped);
}
