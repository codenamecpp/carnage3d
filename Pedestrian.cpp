#include "stdafx.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"

PedestrianControl::PedestrianControl(Pedestrian& pedestrian)
    : mPedestrian(pedestrian)
{
    ResetControl();
}

void PedestrianControl::ResetControl()
{
    mTurnAngle = 0.0f;
    mTurnLeft = false;
    mTurnRight = false;
    mWalkBackward = false;
    mWalkForward = false;
    mRunning = false;
}

void PedestrianControl::SetTurnLeft(bool turnEnabled)
{
    mTurnLeft = turnEnabled;
    mTurnAngle = 0.0f;
}

void PedestrianControl::SetTurnRight(bool turnEnabled)
{
    mTurnRight = turnEnabled;
    mTurnAngle = 0.0f;
}

void PedestrianControl::SetTurnAngle(float turnAngle)
{
    mTurnAngle = turnAngle;
    mTurnLeft = false;
    mTurnRight = false;
}

void PedestrianControl::SetWalkForward(bool walkEnabled)
{
    mWalkForward = walkEnabled;
}

void PedestrianControl::SetWalkBackward(bool walkEnabled)
{
    mWalkBackward = walkEnabled;
}

bool PedestrianControl::IsTurnAround() const
{
    return mTurnLeft || mTurnRight || fabs(mTurnAngle) > 0.01f;
}

bool PedestrianControl::IsMoves() const
{
    return mWalkBackward || mWalkForward;
}

void PedestrianControl::SetRunning(bool runEnabled)
{
    mRunning = runEnabled;
}

//////////////////////////////////////////////////////////////////////////

Pedestrian::Pedestrian(unsigned int id)
    : mPhysicalBody()
    , mDead()
    , mCurrentAnimID(eSpriteAnimationID_Null)
    , mControl(*this)
    , mID(id)
    , mMarkForDeletion()
    , mActivePedsNode(this)
    , mDeletePedsNode(this)
{
}

Pedestrian::~Pedestrian()
{
    if (mPhysicalBody)
    {
        gPhysics.DestroyPhysicsObject(mPhysicalBody);
    }
}

void Pedestrian::EnterTheGame()
{
    glm::vec3 startPosition;
    
    mPhysicalBody = gPhysics.CreatePedestrianBody(startPosition, 0.0f);
    debug_assert(mPhysicalBody);

    mMarkForDeletion = false;
    mDead = false;
    mLiveTicks = 0;
    mCurrentAnimID = eSpriteAnimationID_Null;
    // set initial state and animation
    SwitchToAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart);
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
    mAnimation.UpdateFrame(deltaTime);

    mLiveTicks += deltaTime;

    // ignore inputs when falling
    if (IsFalling())
    {
        SwitchToAnimation(eSpriteAnimationID_Ped_Falling, eSpriteAnimLoop_FromStart);
        return;
    }

    // try to turn around
    if (mControl.IsTurnAround())
    {
        if (mControl.mTurnLeft || mControl.mTurnRight)
        {
            mPhysicalBody->SetAngularVelocity((mControl.mTurnLeft ? -1.0f : 1.0f) * gGameRules.mPedestrianTurnSpeed);
        }
        else // specific angle
        {
            float anglePerFrame = anglePerFrame = (mControl.mTurnAngle * gGameRules.mPedestrianTurnSpeed * deltaTime.ToSeconds());
            mControl.mTurnAngle -= anglePerFrame; // todo
        }        
    }
    else
    {
        // stop rotation
        mPhysicalBody->SetAngularVelocity(0.0f);
    }
    // try walk
    if (mControl.IsMoves())
    {
        float moveSpeed = 0.0f;
        bool moveBackward = false;
        if (mControl.mWalkForward)
        {
            if (mControl.mRunning)
            {
                moveSpeed = gGameRules.mPedestrianRunSpeed;
                SwitchToAnimation(eSpriteAnimationID_Ped_Run, eSpriteAnimLoop_FromStart);
            }
            else
            {
                moveSpeed = gGameRules.mPedestrianWalkSpeed;
                SwitchToAnimation(eSpriteAnimationID_Ped_Walk, eSpriteAnimLoop_FromStart);
            }
        }
        else if (mControl.mWalkBackward)
        {
            moveSpeed = gGameRules.mPedestrianWalkSpeed;
            moveBackward = true;
            SwitchToAnimation(eSpriteAnimationID_Ped_Walk, eSpriteAnimLoop_FromStart); // todo:reverse
        }
        // get current direction
        float angleRadians = mPhysicalBody->GetAngleRadians();
        glm::vec3 signVector 
        {
            cos(angleRadians), 0.0f, sin(angleRadians)
        };

        if (moveBackward)
        {
            signVector = -signVector;
        }
        mPhysicalBody->SetLinearVelocity(signVector * moveSpeed);
    }
    else
    {
        mPhysicalBody->SetLinearVelocity({}); // force stop
        SwitchToAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart);
    }
}

void Pedestrian::SwitchToAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode)
{
    if (mCurrentAnimID != animation)
    {
        mAnimation.SetNull();
        if (!gGameMap.mStyleData.GetSpriteAnimation(animation, mAnimation.mAnimData)) // todo
        {
            debug_assert(false);
        }
        mCurrentAnimID = animation;
    }
    mAnimation.PlayAnimation(loopMode);
}

void Pedestrian::SetHeading(float rotationDegrees)
{
    debug_assert(mPhysicalBody);
    mPhysicalBody->SetAngleDegrees(rotationDegrees);
}

void Pedestrian::SetPosition(const glm::vec3& position)
{
    debug_assert(mPhysicalBody);

    mPhysicalBody->SetPosition(position);
}

bool Pedestrian::IsFalling() const
{
    debug_assert(mPhysicalBody);

    return mPhysicalBody->mFalling;
}

//////////////////////////////////////////////////////////////////////////

bool PedestrianManager::Initialize()
{
    mIDsCounter = 0;
    return true;
}

void PedestrianManager::Deinit()
{
    DestroyPedsInList(mActivePedestriansList);
    DestroyPedsInList(mDeletePedestriansList);
}

void PedestrianManager::UpdateFrame(Timespan deltaTime)
{
    DestroyPendingPeds();
    
    bool hasDeletePeds = false;
    for (Pedestrian* currentPed: mActivePedestriansList) // warning: dont add or remove peds during this loop
    {
        if (!currentPed->mMarkForDeletion)
        {
            debug_assert(!mDeletePedestriansList.contains(&currentPed->mDeletePedsNode));
            currentPed->UpdateFrame(deltaTime);
        }

        if (currentPed->mMarkForDeletion)
        {
            mDeletePedestriansList.insert(&currentPed->mDeletePedsNode);
            hasDeletePeds = true;
        }
    }

    if (!hasDeletePeds)
        return;

    // deactivate all peds marked for deletion
    for (Pedestrian* deletePed: mDeletePedestriansList)
    {
        RemoveFromActiveList(deletePed);
    }
}

void PedestrianManager::DestroyPendingPeds()
{
    DestroyPedsInList(mDeletePedestriansList);
}

void PedestrianManager::DestroyPedestrian(Pedestrian* pedestrian)
{
    debug_assert(pedestrian);
    if (pedestrian == nullptr)
        return;

    if (mDeletePedestriansList.contains(&pedestrian->mDeletePedsNode))
    {
        mDeletePedestriansList.remove(&pedestrian->mDeletePedsNode);
    }

    if (mActivePedestriansList.contains(&pedestrian->mActivePedsNode))
    {
        mActivePedestriansList.remove(&pedestrian->mActivePedsNode);
    }

    mPedsPool.destroy(pedestrian);
}

Pedestrian* PedestrianManager::CreateRandomPed(const glm::vec3& position)
{
    unsigned int pedestrianID = GenerateUniqueID();

    Pedestrian* instance = mPedsPool.create(pedestrianID);
    debug_assert(instance);

    AddToActiveList(instance);

    // init
    instance->EnterTheGame();
    instance->mPhysicalBody->SetPosition(position);
    return instance;
}

void PedestrianManager::AddToActiveList(Pedestrian* pedestrian)
{
    debug_assert(pedestrian);
    if (pedestrian == nullptr)
        return;

    debug_assert(!mActivePedestriansList.contains(&pedestrian->mActivePedsNode));
    debug_assert(!mDeletePedestriansList.contains(&pedestrian->mDeletePedsNode));
    mActivePedestriansList.insert(&pedestrian->mActivePedsNode);
}

void PedestrianManager::RemoveFromActiveList(Pedestrian* pedestrian)
{
    debug_assert(pedestrian);
    if (pedestrian && mActivePedestriansList.contains(&pedestrian->mActivePedsNode))
    {
        mActivePedestriansList.remove(&pedestrian->mActivePedsNode);
    }
}

unsigned int PedestrianManager::GenerateUniqueID()
{
    unsigned int newID = ++mIDsCounter;
    if (newID == 0) // overflow
    {
        debug_assert(false);
    }
    return newID;
}

void PedestrianManager::DestroyPedsInList(cxx::intrusive_list<Pedestrian>& pedsList)
{
    while (pedsList.has_elements())
    {
        cxx::intrusive_node<Pedestrian>* pedestrianNode = pedsList.get_head_node();
        pedsList.remove(pedestrianNode);

        Pedestrian* pedestrian = pedestrianNode->get_element();
        mPedsPool.destroy(pedestrian);
    }
}
