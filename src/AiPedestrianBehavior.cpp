#include "stdafx.h"
#include "AiPedestrianBehavior.h"
#include "Pedestrian.h"
#include "AiCharacterController.h"
#include "MapDirection2D.h"
#include "GameMapManager.h"
#include "CarnageGame.h"

//////////////////////////////////////////////////////////////////////////

AiPedestrianBehavior::AiActivity::AiActivity(AiPedestrianBehavior* aiBehavior)
    : mAiBehavior(aiBehavior)
{
    debug_assert(mAiBehavior);
}

AiPedestrianBehavior::AiActivity::~AiActivity()
{
    // do nothing
}

void AiPedestrianBehavior::AiActivity::StartActivity()
{
    if (!IsStatusInProgress())
    {
        SetActivityStatus(eAiActivityStatus_InProgress);
        OnActivityStart();
    }
    else
    {
        debug_assert(false);
    }
}

void AiPedestrianBehavior::AiActivity::UpdateActivity()
{
    if (IsStatusInProgress())
    {
        OnActivityUpdate();      
    }
}

void AiPedestrianBehavior::AiActivity::CancelActivity()
{
    if (IsStatusInProgress())
    {
        SetActivityStatus(eAiActivityStatus_Cancelled);
        OnActivityCancelled();
    }
}

bool AiPedestrianBehavior::AiActivity::IsStatusInProgress() const
{
    return mActivityStatus == eAiActivityStatus_InProgress;
}

bool AiPedestrianBehavior::AiActivity::IsStatusSuccess() const
{
    return mActivityStatus == eAiActivityStatus_Success;
}

bool AiPedestrianBehavior::AiActivity::IsStatusFailed() const
{
    return mActivityStatus == eAiActivityStatus_Failed;
}

bool AiPedestrianBehavior::AiActivity::IsStatusCancelled() const
{
    return mActivityStatus == eAiActivityStatus_Cancelled;
}

void AiPedestrianBehavior::AiActivity::SetActivityStatus(eAiActivityStatus newStatus)
{
    if (newStatus == eAiActivityStatus_Failed)
    {
        int bp = 0;
    }
    mActivityStatus = newStatus;
}

//////////////////////////////////////////////////////////////////////////

AiPedestrianBehavior::AiActiviy_Wanter::AiActiviy_Wanter(AiPedestrianBehavior* aiBehavior)
    : AiActivity(aiBehavior)
{
}

void AiPedestrianBehavior::AiActiviy_Wanter::OnActivityStart()
{
    mAiBehavior->StopCharacter();

    if (!ChooseDesiredPoint(eGroundType_Pawement))
    {   
        SetActivityStatus(eAiActivityStatus_Failed);
        return;
    }
}

void AiPedestrianBehavior::AiActiviy_Wanter::OnActivityUpdate()
{
    mAiBehavior->StopCharacter();

    Pedestrian* character = mAiBehavior->GetCharacter();
    debug_assert(character);

    const float ArriveDistance = gGameParams.mPedestrianBoundsSphereRadius * 2.0f;
    if (!mAiBehavior->MoveCharacterTowardsDesiredPoint(ArriveDistance, false))
        return; // not arrived yet

    // arrived to destination, choose next position
    if (ChooseDesiredPoint(eGroundType_Pawement))
    {
        mAiBehavior->MoveCharacterTowardsDesiredPoint(ArriveDistance, false);
    }
    else
    {
        SetActivityStatus(eAiActivityStatus_Failed); // fail to arrive
    }
}

bool AiPedestrianBehavior::AiActiviy_Wanter::ChooseDesiredPoint(eGroundType groundType)
{
    Pedestrian* character = mAiBehavior->GetCharacter();
    debug_assert(character);

    cxx::angle_t currHeading = character->mTransform.mOrientation;

    // choose new block ir next order: forward, left, right, backward
    const eMapDirection2D currentMapDirection = GetStraightMapDirectionFromHeading(currHeading);
    const eMapDirection2D moveDirs[] =
    {
        currentMapDirection,
        GetStraightMapDirectionCCW(currentMapDirection),
        GetStraightMapDirectionCW(currentMapDirection),
        GetStraightMapDirectionOpposite(currentMapDirection)
    };

    glm::ivec3 logPosition = Convert::MetersToMapUnits(character->mTransform.mPosition);
    const MapBlockInfo* currentBlock = gGameMap.GetBlockInfo(logPosition.x, logPosition.z, logPosition.y);
    eMapDirection2D bestDirection = eMapDirection2D_None;
    for (eMapDirection2D directionCandidate: moveDirs)
    {
        const MapBlockInfo* neighbourBlock = gGameMap.GetNeighbourBlock(logPosition.x, logPosition.z, logPosition.y, directionCandidate);
        if ((neighbourBlock->mGroundType == groundType) || (neighbourBlock->mGroundType == currentBlock->mGroundType))
        {
            bestDirection = directionCandidate;
            break;
        }
    }

    // nothing found
    if (bestDirection == eMapDirection2D_None)
        return false;

    debug_assert(IsMapDirectionStraight(bestDirection));

    switch (bestDirection)
    {
        case eMapDirection2D_N: logPosition.z -= 1; break;
        case eMapDirection2D_E: logPosition.x += 1; break;
        case eMapDirection2D_S: logPosition.z += 1; break;
        case eMapDirection2D_W: logPosition.x -= 1; break;
    }

    // choose random point within block
    float randomSubPosx = gCarnageGame.mGameRand.generate_float(0.1f, 0.9f);
    float randomSubPosy = gCarnageGame.mGameRand.generate_float(0.1f, 0.9f);
    mAiBehavior->mDesiredPoint.x = Convert::MapUnitsToMeters(logPosition.x * 1.0f) + Convert::MapUnitsToMeters(randomSubPosx);
    mAiBehavior->mDesiredPoint.y = Convert::MapUnitsToMeters(logPosition.z * 1.0f) + Convert::MapUnitsToMeters(randomSubPosy);
    return true;
}

//////////////////////////////////////////////////////////////////////////

AiPedestrianBehavior::AiActivity_Runaway::AiActivity_Runaway(AiPedestrianBehavior* aiBehavior)
    : AiActiviy_Wanter(aiBehavior)
{
}

void AiPedestrianBehavior::AiActivity_Runaway::OnActivityStart()
{
    mAiBehavior->StopCharacter();

    if (!ChooseRunawayPoint())
    {   
        SetActivityStatus(eAiActivityStatus_Failed);
        return;
    }
}

void AiPedestrianBehavior::AiActivity_Runaway::OnActivityUpdate()
{
    mAiBehavior->StopCharacter();

    Pedestrian* character = mAiBehavior->GetCharacter();
    debug_assert(character);

    const float ArriveDistance = gGameParams.mPedestrianBoundsSphereRadius * 2.0f;
    if (!mAiBehavior->MoveCharacterTowardsDesiredPoint(ArriveDistance, true))
        return; // not arrived yet

    // arrived to destination, choose next position
    if (ChooseRunawayPoint())
    {
        mAiBehavior->MoveCharacterTowardsDesiredPoint(ArriveDistance, true);
    }
    else
    {
        SetActivityStatus(eAiActivityStatus_Failed); // fail to arrive
    }
}

bool AiPedestrianBehavior::AiActivity_Runaway::ChooseRunawayPoint()
{
    if (ChooseDesiredPoint(eGroundType_Pawement) || ChooseDesiredPoint(eGroundType_Field) ||
        ChooseDesiredPoint(eGroundType_Road) || ChooseDesiredPoint(eGroundType_Air))
    {
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

AiPedestrianBehavior::AiActivity_FollowLeader::AiActivity_FollowLeader(AiPedestrianBehavior* aiBehavior)
    : AiActivity(aiBehavior)
{
}

void AiPedestrianBehavior::AiActivity_FollowLeader::OnActivityStart()
{
    mAiBehavior->StopCharacter();

    if (!CheckCanFollowTheLeader())
    {
        SetActivityStatus(eAiActivityStatus_Failed);
        return;
    }
}

void AiPedestrianBehavior::AiActivity_FollowLeader::OnActivityUpdate()
{
    mAiBehavior->StopCharacter();

    if (!CheckCanFollowTheLeader())
    {
        SetActivityStatus(eAiActivityStatus_Failed);
        return;
    }

    const float IdleDistance = gGameParams.mPedestrianBoundsSphereRadius * 4.0f;
    const float ApproachDistance = gGameParams.mPedestrianBoundsSphereRadius * 2.0f;
    const float RushDistance = ApproachDistance * 4.0f;

    Pedestrian* currCharacter = mAiBehavior->GetCharacter();
    Pedestrian* leadCharacter = mAiBehavior->GetLeader();
    // update desired point
    mAiBehavior->mDesiredPoint = leadCharacter->mTransform.GetPosition2();
    if (currCharacter->IsStanding())
    {
        mAiBehavior->MoveCharacterTowardsDesiredPoint(IdleDistance, false);
    }
    else if (currCharacter->IsWalking())
    {
        float distanceToTarget2 = glm::distance2(currCharacter->mTransform.GetPosition2(), mAiBehavior->mDesiredPoint);
        // check whether in rush
        bool inRush = leadCharacter->IsRunning() || (distanceToTarget2 > (RushDistance * RushDistance));
        mAiBehavior->MoveCharacterTowardsDesiredPoint(ApproachDistance, inRush);
    }
}

bool AiPedestrianBehavior::AiActivity_FollowLeader::CheckCanFollowTheLeader() const
{
    Pedestrian* character = mAiBehavior->GetCharacter();
    debug_assert(character);

    Pedestrian* leaderCharacter = mAiBehavior->GetLeader();
    if ((leaderCharacter == nullptr) || leaderCharacter->IsDead() || leaderCharacter->IsDies())
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////

AiPedestrianBehavior::AiPedestrianBehavior(AiCharacterController* aiController, eAiPedestrianBehavior behaviorID)
    : mAiController(aiController)
    , mBehaviorID(behaviorID)
    , mDesiredPoint()
    , mActivity_Wander(this)
    , mActivity_Runaway(this)
    , mActivity_FollowLeader(this)
{
    debug_assert(mAiController);
}

AiPedestrianBehavior::~AiPedestrianBehavior()
{
}

void AiPedestrianBehavior::ActivateBehavior()
{
    debug_assert(mAiController);

    OnActivateBehavior();

    debug_assert(mCurrentActivity == nullptr);
    mDesiredActivity = nullptr;
}

void AiPedestrianBehavior::ShutdownBehavior()
{
    if (mCurrentActivity && mCurrentActivity->IsStatusInProgress())
    {
        mCurrentActivity->CancelActivity();
    }

    OnShutdownBehavior();

    // reset state
    mCurrentActivity = nullptr;
    mDesiredActivity = nullptr;
}

void AiPedestrianBehavior::UpdateBehavior()
{
    ScanForThreats();
    ScanForLeader();

    ChooseDesiredActivity();

    if (mDesiredActivity && (mCurrentActivity != mDesiredActivity))
    {
        if (mCurrentActivity && mCurrentActivity->IsStatusInProgress())
        {
            mCurrentActivity->CancelActivity();
            mCurrentActivity = nullptr;
        }
        mCurrentActivity = mDesiredActivity;
        mDesiredActivity = nullptr;
        mCurrentActivity->StartActivity();
    }
    else if (mCurrentActivity)
    {
        mCurrentActivity->UpdateActivity();
    }
}

void AiPedestrianBehavior::ChangeMemoryBits(AiBehaviorMemoryBits enableBits, AiBehaviorMemoryBits disableBits)
{
    mMemoryBits = (mMemoryBits | enableBits) & ~disableBits;
}

bool AiPedestrianBehavior::CheckMemoryBits(AiBehaviorMemoryBits memoryBits) const
{
    return (mMemoryBits & memoryBits) > 0;
}

bool AiPedestrianBehavior::CheckMemoryBitsAll(AiBehaviorMemoryBits memoryBits) const
{
    return (mMemoryBits & memoryBits) == memoryBits;
}

void AiPedestrianBehavior::ChangeBehaviorBits(AiBehaviorBits enableBits, AiBehaviorBits disableBits)
{
    mBehaviorBits = (mBehaviorBits | enableBits) & ~disableBits;
}

bool AiPedestrianBehavior::CheckBehaviorBits(AiBehaviorBits bits) const
{
    return (mBehaviorBits & bits) > 0;
}

bool AiPedestrianBehavior::CheckBehaviorBitsAll(AiBehaviorBits bits) const
{
    return (mBehaviorBits & bits) == bits;
}

Pedestrian* AiPedestrianBehavior::GetCharacter() const
{
    if (mAiController)
        return mAiController->mCharacter;

    return nullptr;
}

Pedestrian* AiPedestrianBehavior::GetLeader() const
{
    return mLeader;
}

void AiPedestrianBehavior::SetLeader(Pedestrian* pedestrian)
{
    // sanity checks
    if ((pedestrian == nullptr) || (pedestrian == GetCharacter()))
    {
        debug_assert(false);
        return;
    }

    mLeader = pedestrian;
}

void AiPedestrianBehavior::ChooseDesiredActivity()
{
    if (CheckMemoryBits(AiBehaviorMemoryBits_InPanic))
    {
        mDesiredActivity = &mActivity_Runaway;
        return;
    }

    Pedestrian* character = GetCharacter();
    debug_assert(character);
    if (character->HasFear_Explosions() && CheckMemoryBits(AiBehaviorMemoryBits_HearExplosion))
    {
        ChangeMemoryBits(AiBehaviorMemoryBits_InPanic, AiBehaviorMemoryBits_None);
        mDesiredActivity = &mActivity_Runaway;
        return;
    }

    if (character->HasFear_GunShots() && CheckMemoryBits(AiBehaviorMemoryBits_HearGunShots))
    {
        ChangeMemoryBits(AiBehaviorMemoryBits_InPanic, AiBehaviorMemoryBits_None);
        mDesiredActivity = &mActivity_Runaway;
        return;
    }

    if (mLeader)
    {
        mDesiredActivity = &mActivity_FollowLeader;
        return;
    }

    mDesiredActivity = &mActivity_Wander;
}

bool AiPedestrianBehavior::MoveCharacterTowardsDesiredPoint(float minArriveDistance, bool setRunning)
{
    Pedestrian* character = GetCharacter();
    debug_assert(character);
    if (character)
    {
        mAiController->mCtlState.Clear();

        float tolerance2 = glm::pow(minArriveDistance * minArriveDistance, 2.0f);

        glm::vec2 currentPos2 = character->mTransform.GetPosition2();
        if (glm::distance2(currentPos2, mDesiredPoint) <= tolerance2)
            return true;

        // setup sign direction
        glm::vec2 toTarget = glm::normalize(mDesiredPoint - currentPos2);
        mAiController->mCtlState.mDesiredRotationAngle = cxx::angle_t::from_radians(::atan2f(toTarget.y, toTarget.x));
        mAiController->mCtlState.mRotateToDesiredAngle = true;
        mAiController->mCtlState.mWalkForward = true;
        mAiController->mCtlState.mRun = setRunning;
    }
    return false;
}

void AiPedestrianBehavior::StopCharacter()
{
    mAiController->mCtlState.Clear();
}

void AiPedestrianBehavior::ScanForThreats()
{
    // reset previous state
    ChangeMemoryBits(AiBehaviorMemoryBits_None, AiBehaviorMemoryBits_HearExplosion | AiBehaviorMemoryBits_HearGunShots);

    Pedestrian* character = GetCharacter();
    if (CheckMemoryBits(AiBehaviorMemoryBits_InPanic) || character->IsDead() || character->IsDies())
        return;

    AiBehaviorMemoryBits enableMemoryBits = AiBehaviorMemoryBits_None;
    BroadcastEvent eventData;
    glm::vec2 characterPos2 = character->mTransform.GetPosition2();
    // check gunshots
    {
        for (BroadcastEventsIterator eventsIter;;)
        {
            if (!eventsIter.NextEventInDistance(eBroadcastEvent_GunShot, characterPos2, gGameParams.mAiReactOnGunshotsDistance, eventData))
                break;

            if (eventData.mCharacter == character)// hear own gunshots
                continue;

            enableMemoryBits = (enableMemoryBits | AiBehaviorMemoryBits_HearGunShots);
            break;
        }
    }

    // check explosions
    {
        for (BroadcastEventsIterator eventsIter;;)
        {
            if (!eventsIter.NextEventInDistance(eBroadcastEvent_Explosion, characterPos2, gGameParams.mAiReactOnExplosionsDistance, eventData))
                break;

            enableMemoryBits = (enableMemoryBits | AiBehaviorMemoryBits_HearExplosion);
            break;
        }
    }

    if (enableMemoryBits)
    {
        ChangeMemoryBits(enableMemoryBits, AiBehaviorMemoryBits_None);
    }
}

void AiPedestrianBehavior::ScanForLeader()
{
    if (mLeader)
    {
        // check if leader still alive
        if (!mLeader->IsDead() && !mLeader->IsDies())
            return;

        mLeader.reset();
    }

    Pedestrian* bestHumanCharacter = nullptr;
    Pedestrian* character = GetCharacter();
    if (CheckMemoryBits(AiBehaviorMemoryBits_InPanic) || character->IsDead() || character->IsDies())
        return;

    if (!CheckBehaviorBits(AiBehaviorBits_PlayerFollower))
        return;

    glm::vec2 currPosition2 = character->mTransform.GetPosition2();

    // try follow human character Nearby
    float maxSignDistance = Convert::MapUnitsToMeters(0.5f);
    float bestDistance2 = glm::pow(maxSignDistance, 2.0f);
    for (HumanPlayer* currentPlayer: gCarnageGame.mHumanPlayers)
    {
        if ((currentPlayer == nullptr) || (character == currentPlayer->mCharacter))
            continue;

        if (!currentPlayer->mCharacter->IsStanding())
            continue;

        float currDistance2 = glm::distance2(currentPlayer->mCharacter->mTransform.GetPosition2(), currPosition2);
        if (currDistance2 > bestDistance2)
            continue;

        bestDistance2 = currDistance2;
        bestHumanCharacter = currentPlayer->mCharacter;
    }

    if (bestHumanCharacter)
    {
        mLeader = bestHumanCharacter;
    }
}
