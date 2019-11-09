#include "stdafx.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "SpriteManager.h"
#include "RenderingManager.h"
#include "PedestrianStates.h"
#include "Vehicle.h"

Pedestrian::Pedestrian(GameObjectID id) : GameObject(eGameObjectType_Pedestrian, id)
    , mPhysicsComponent()
    , mCurrentAnimID(eSpriteAnimID_Null)
    , mController()
    , mDrawHeight()
    , mPedsListNode(this)
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

    mCurrentStateTime = 0;
    mWeaponRechargeTime = 0;
    mRemapIndex = NO_REMAP;

    // reset actions
    for (int iaction = 0; iaction < ePedestrianAction_COUNT; ++iaction)
    {
        mCtlActions[iaction] = false;
    }

    // reset weapon ammo
    for (int iweapon = 0; iweapon < eWeaponType_COUNT; ++iweapon)
    {
        mWeaponsAmmo[iweapon] = -1; // temporary
    }
    mWeaponsAmmo[eWeaponType_Fists] = -1;
    mCurrentWeapon = eWeaponType_Fists;
    
    mPhysicsComponent = gPhysics.CreatePhysicsComponent(this, startPosition, cxx::angle_t::from_degrees(0.0f));
    debug_assert(mPhysicsComponent);

    mCurrentAnimID = eSpriteAnimID_Null;
    mDeathReason = ePedestrianDeathReason_null;

    mCurrentCar = nullptr;
    mCurrentSeat = eCarSeat_Any;

    ChangeState(&mStateStandingStill, nullptr);
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
    // update controller logic if it specified
    if (mController)
    {
        mController->UpdateFrame(this, deltaTime);
    }
    mCurrentAnimState.AdvanceAnimation(deltaTime);

    mCurrentStateTime += deltaTime;
    // update current state logic
    if (mCurrentState)
    {
        mCurrentState->ProcessStateFrame(this, deltaTime);
    }
}

void Pedestrian::DrawFrame(SpriteBatch& spriteBatch)
{
    glm::vec3 position = mPhysicsComponent->GetPosition();
    ComputeDrawHeight(position);

    ePedestrianState currState = GetCurrentStateID();
    if (currState == ePedestrianState_DrivingCar)
    {
        // dont draw pedestrian if it in car with hard top
        if (mDrawHeight < mCurrentCar->mDrawHeight)
            return;
    }

    cxx::angle_t rotationAngle = mPhysicsComponent->GetRotationAngle() - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);

    int spriteLinearIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Ped, mCurrentAnimState.GetCurrentFrame());

    int remapClut = mRemapIndex == NO_REMAP ? 0 : mRemapIndex + gGameMap.mStyleData.GetPedestrianRemapsBaseIndex();
    gSpriteManager.GetSpriteTexture(mObjectID, spriteLinearIndex, remapClut, mDrawSprite);

    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mDrawSprite.mScale = SPRITE_SCALE;
    mDrawSprite.mRotateAngle = rotationAngle;
    mDrawSprite.mHeight = mDrawHeight;
    mDrawSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mDrawSprite);
}

void Pedestrian::DrawDebug(DebugRenderer& debugRender)
{
    glm::vec3 position = mPhysicsComponent->GetPosition();

    glm::vec2 signVector = mPhysicsComponent->GetSignVector() * gGameParams.mPedestrianSpotTheCarDistance;

    debugRender.DrawLine(position, position + glm::vec3(signVector.x, 0.0f, signVector.y), COLOR_WHITE);
}

void Pedestrian::ComputeDrawHeight(const glm::vec3& position)
{
    if (IsCarPassenger())
    {
        debug_assert(mCurrentCar);
        bool isBike = (mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
        // dont draw pedestrian if it in car with hard top
        if ((mCurrentCar->mCarStyle->mConvertible == eCarConvertible_HardTop ||
            mCurrentCar->mCarStyle->mConvertible == eCarConvertible_HardTopAnimated) && !isBike)
        {
            mDrawHeight = mCurrentCar->mDrawHeight - 0.01f; // todo: magic numbers
        }
        else
        {
            mDrawHeight = mCurrentCar->mDrawHeight + 0.01f; // todo: magic numbers
        }
        return;
    }

    float halfBox = PED_SPRITE_DRAW_BOX_SIZE * 0.5f;

    //glm::vec3 points[4] = {
    //    { 0.0f,     position.y + 0.01f, -halfBox },
    //    { halfBox,  position.y + 0.01f, 0.0f },
    //    { 0.0f,     position.y + 0.01f, halfBox },
    //    { -halfBox, position.y + 0.01f, 0.0f },
    //};

    glm::vec3 points[4] = {
        { -halfBox, position.y + 0.01f, -halfBox },
        { halfBox,  position.y + 0.01f, -halfBox },
        { halfBox,  position.y + 0.01f, halfBox },
        { -halfBox, position.y + 0.01f, halfBox },
    };

    float maxHeight = position.y;
    for (glm::vec3& currPoint: points)
    {
        //currPoint = glm::rotate(currPoint, angleRadians, glm::vec3(0.0f, -1.0f, 0.0f)); // dont rotate for peds
        currPoint.x += position.x;
        currPoint.z += position.z;

        // get height
        float height = gGameMap.GetHeightAtPosition(currPoint);
        if (height > maxHeight)
        {
            maxHeight = height;
        }
    }

    // todo: get rid of magic numbers
    if (GetCurrentStateID() == ePedestrianState_SlideOnCar)
    {
        maxHeight += 0.35f; // todo: magic numbers
    }

    float drawOffset = 0.02f; // todo: magic numbers
    if (IsUnconscious())
    {
        drawOffset = 0.001f; // todo: magic numbers
    }

    mDrawHeight = maxHeight + drawOffset;
}

void Pedestrian::SetAnimation(eSpriteAnimID animation, eSpriteAnimLoop loopMode)
{
    if (mCurrentAnimID != animation)
    {
        mCurrentAnimState.SetNull();
        if (!gGameMap.mStyleData.GetSpriteAnimation(animation, mCurrentAnimState.mAnimDesc)) // todo
        {
            debug_assert(false);
        }
        mCurrentAnimID = animation;
    }
    mCurrentAnimState.PlayAnimation(loopMode);
}

void Pedestrian::ChangeState(PedestrianBaseState* nextState, const PedestrianStateEvent* transitionEvent)
{
    if (nextState == mCurrentState && mCurrentState)
        return;
    mCurrentStateTime = 0;
    debug_assert(nextState);
    if (mCurrentState)
    {
        mCurrentState->ProcessStateExit(this, transitionEvent);
    }
    mCurrentState = nextState;
    if (mCurrentState)
    {
        mCurrentState->ProcessStateEnter(this, transitionEvent);
    }
}

bool Pedestrian::ProcessEvent(const PedestrianStateEvent& eventData)
{
    return mCurrentState && mCurrentState->ProcessStateEvent(this, eventData);
}

ePedestrianState Pedestrian::GetCurrentStateID() const
{
    if (mCurrentState == nullptr)
        return ePedestrianState_Unspecified;

    return mCurrentState->GetStateID();
}

bool Pedestrian::IsCarPassenger() const
{
    ePedestrianState currState = GetCurrentStateID();
    if (currState == ePedestrianState_DrivingCar || currState == ePedestrianState_EnteringCar || 
        currState == ePedestrianState_ExitingCar)
    {
        return true; // includes driver
    }
    return false;
}

bool Pedestrian::IsCarDriver() const
{
    ePedestrianState currState = GetCurrentStateID();
    if (currState == ePedestrianState_DrivingCar || currState == ePedestrianState_EnteringCar || 
        currState == ePedestrianState_ExitingCar)
    {
        return mCurrentSeat == eCarSeat_Driver;
    }
    return false;
}

bool Pedestrian::IsStanding() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_StandingStill || currState == ePedestrianState_StandsAndShoots);
}

bool Pedestrian::IsShooting() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_StandsAndShoots || currState == ePedestrianState_WalksAndShoots ||
        currState == ePedestrianState_RunsAndShoots);
}

bool Pedestrian::IsWalking() const
{
    ePedestrianState currState = GetCurrentStateID();
    return (currState == ePedestrianState_Walks || currState == ePedestrianState_Runs || 
        currState == ePedestrianState_WalksAndShoots || currState == ePedestrianState_RunsAndShoots);
}

bool Pedestrian::IsUnconscious() const
{
    ePedestrianState currState = GetCurrentStateID();
    return currState == ePedestrianState_KnockedDown;
}

bool Pedestrian::IsDead() const
{
    ePedestrianState currState = GetCurrentStateID();
    return currState == ePedestrianState_Dead;
}

void Pedestrian::SetCarEntered(Vehicle* targetCar, eCarSeat targetSeat)
{
    debug_assert(mCurrentCar == nullptr);
    debug_assert(targetCar && targetSeat != eCarSeat_Any);

    mCurrentCar = targetCar;
    mCurrentSeat = targetSeat;

    // reset actions
    mCtlActions[ePedestrianAction_TurnLeft] = false;
    mCtlActions[ePedestrianAction_TurnRight] = false;
    mCtlActions[ePedestrianAction_Jump] = false;
    mCtlActions[ePedestrianAction_WalkForward] = false;
    mCtlActions[ePedestrianAction_WalkBackward] = false;
    mCtlActions[ePedestrianAction_Run] = false;
    mCtlActions[ePedestrianAction_Shoot] = false;
}

void Pedestrian::SetCarExited()
{
    mCurrentCar = nullptr;

    // reset actions
    mCtlActions[ePedestrianAction_HandBrake] = false;
    mCtlActions[ePedestrianAction_Accelerate] = false;
    mCtlActions[ePedestrianAction_Reverse] = false;
    mCtlActions[ePedestrianAction_SteerLeft] = false;
    mCtlActions[ePedestrianAction_SteerRight] = false;
    mCtlActions[ePedestrianAction_Horn] = false;
}

void Pedestrian::SetCurrentWeapon(eWeaponType weapon)
{
    mCurrentWeapon = weapon;
}

void Pedestrian::SetDead(ePedestrianDeathReason deathReason)
{
    debug_assert(mDeathReason == ePedestrianDeathReason_null);
    debug_assert(deathReason != ePedestrianDeathReason_null);
    mDeathReason = deathReason;
}

