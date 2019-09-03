#include "stdafx.h"
#include "Pedestrian.h"

Pedestrian::Pedestrian()
    : mPosition()
    , mPrevPosition()
    , mRotation()
    , mPrevRotation()
    , mSphereRadius(1.0f)
    , mDead()
    , mVelocity()
    , mCurrentAnimID(eSpriteAnimationID_Null)
{
}

void Pedestrian::EnterTheGame()
{
    mDead = false;
    mLiveTicks = 0;
    mVelocity = {0.0f, 0.0f, 0.0f};
    mCurrentAnimID = eSpriteAnimationID_Null;
    // set initial state and animation
    SwitchToAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart);
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
    mAnimation.UpdateFrame(deltaTime);

    mLiveTicks += deltaTime;
}

void Pedestrian::SwitchToAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode)
{
    if (mCurrentAnimID != animation)
    {
        mAnimation.SetNull();
        if (!gCarnageGame.mCityScape.mStyleData.GetSpriteAnimation(animation, mAnimation.mAnimData)) // todo
        {
            debug_assert(false);
        }
        mCurrentAnimID = animation;
    }
    mAnimation.PlayAnimation(loopMode);
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
    instance->EnterTheGame();
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
