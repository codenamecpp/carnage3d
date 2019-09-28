#include "stdafx.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"

Pedestrian::Pedestrian(unsigned int id)
    : GameObject(id)
    , mPhysicsComponent()
    , mDead()
    , mCurrentAnimID(eSpriteAnimationID_Null)
    , mControl(*this)
    , mActivePedsNode(this)
    , mDeletePedsNode(this)
{
}

Pedestrian::~Pedestrian()
{
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
}

void Pedestrian::EnterTheGame()
{
    glm::vec3 startPosition;
    
    mPhysicsComponent = gPhysics.CreatePedPhysicsComponent(startPosition, 0.0f);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
    mCurrentAnimID = eSpriteAnimationID_Null;
    // set initial state and animation
    SwitchToAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart);
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
    mAnimation.AdvanceAnimation(deltaTime);

    // ignore inputs when falling
    if (IsFalling())
    {
        SwitchToAnimation(eSpriteAnimationID_Ped_Falling, eSpriteAnimLoop_FromStart);
        return;
    }

    // try to turn around
    if (mControl.IsTurnAround())
    {
        mPhysicsComponent->SetAngularVelocity((mControl.mTurnLeft ? -1.0f : 1.0f) * gGameRules.mPedestrianTurnSpeed);
    }
    else
    {
        // stop rotation
        mPhysicsComponent->SetAngularVelocity(0.0f);
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
        float angleRadians = mPhysicsComponent->GetAngleRadians();
        glm::vec2 signVector 
        {
            cos(angleRadians), sin(angleRadians)
        };

        if (moveBackward)
        {
            signVector = -signVector;
        }
        mPhysicsComponent->SetLinearVelocity(signVector * moveSpeed);
    }
    else
    {
        mPhysicsComponent->SetLinearVelocity({}); // force stop
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
    debug_assert(mPhysicsComponent);
    mPhysicsComponent->SetAngleDegrees(rotationDegrees);
}

void Pedestrian::SetPosition(const glm::vec3& position)
{
    debug_assert(mPhysicsComponent);

    mPhysicsComponent->SetPosition(position);
}

bool Pedestrian::IsFalling() const
{
    debug_assert(mPhysicsComponent);

    return !mPhysicsComponent->mOnTheGround;
}