#include "stdafx.h"
#include "AiCharacterController.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "CarnageGame.h"
#include "DebugRenderer.h"
#include "BroadcastEventsManager.h"

//////////////////////////////////////////////////////////////////////////

// todo: temporary implementation

//////////////////////////////////////////////////////////////////////////

enum eMapDirection
{
    eMapDirection_N,
    eMapDirection_E,
    eMapDirection_S,
    eMapDirection_W
};

inline eMapDirection GetMapDirectionFromHeading(float angleDegrees)
{
    static const std::pair<float, eMapDirection> Directions[] =
    {
        {360.0f, eMapDirection_E},
        {  0.0f, eMapDirection_E},
        { 90.0f, eMapDirection_S},
        {180.0f, eMapDirection_W},
        {270.0f, eMapDirection_N},
    };

    angleDegrees = cxx::normalize_angle_360(angleDegrees);

    for (const auto& curr: Directions)
    {
        if (fabs(curr.first - angleDegrees) <= 45.0f)
            return curr.second;
    }
    debug_assert(false);
    return eMapDirection_E;
}

inline eMapDirection GetMapDirectionCW(eMapDirection dir)
{
    switch (dir)
    {
        case eMapDirection_N: return eMapDirection_E;
        case eMapDirection_E: return eMapDirection_S;
        case eMapDirection_S: return eMapDirection_W;
        case eMapDirection_W: return eMapDirection_N;
    }
    debug_assert(false);
    return eMapDirection_E;
}

inline eMapDirection GetMapDirectionCCW(eMapDirection dir)
{
    switch (dir)
    {
        case eMapDirection_N: return eMapDirection_W;
        case eMapDirection_E: return eMapDirection_N;
        case eMapDirection_S: return eMapDirection_E;
        case eMapDirection_W: return eMapDirection_S;
    }
    debug_assert(false);
    return eMapDirection_E;
}

inline eMapDirection GetMapDirectionOpposite(eMapDirection dir)
{
    switch (dir)
    {
        case eMapDirection_N: return eMapDirection_S;
        case eMapDirection_E: return eMapDirection_W;
        case eMapDirection_S: return eMapDirection_N;
        case eMapDirection_W: return eMapDirection_E;
    }
    debug_assert(false);
    return eMapDirection_E;
}

// Get straight point vector for direction
inline const glm::ivec3& GetVectorFromMapDirection(eMapDirection direction)
{
    static const glm::ivec3 Vecs[] =
    {
        { 0, 0, -1}, // n
        { 1, 0,  0}, // e
        { 0, 0,  1}, // s
        {-1, 0,  0}, // w
    };
    return Vecs[direction];
}

//////////////////////////////////////////////////////////////////////////

AiCharacterController::AiCharacterController(Pedestrian* character)
{
    mCharacter = character;
    mFollowNearDistance = gGameParams.mPedestrianBoundsSphereRadius * 2.0f;
    mFollowFarDistance = Convert::MapUnitsToMeters(0.5f);
    mDefaultNearDistance = gGameParams.mPedestrianBoundsSphereRadius;

    if (mCharacter)
    {
        debug_assert(mCharacter->mController == nullptr);
        mCharacter->mController = this;
    }
}

void AiCharacterController::DebugDraw(DebugRenderer& debugRender)
{
    if (mAiMode == ePedestrianAiMode_None || mAiMode == ePedestrianAiMode_Disabled)
        return;

    glm::vec3 currpos = mCharacter->mPhysicsBody->GetPosition();
    glm::vec3 destpos (mDestinationPoint.x, currpos.y, mDestinationPoint.y);

    debugRender.DrawLine(currpos, destpos, Color32_Red, false);
}

bool AiCharacterController::ScanForThreats()
{
    if (mCharacter->HasFear_GunShots() && ScanForGunshots())
        return true;

    if (mCharacter->HasFear_Explosions() && ScanForExplosions())
        return true;

    return false;
}

bool AiCharacterController::ScanForExplosions()
{
    float reactionDistance2 = glm::pow(gGameParams.mAiReactOnExplosionsDistance, 2.0f);

    BroadcastEvent eventData;
    if (gBroadcastEvents.PeekClosestEvent(eBroadcastEvent_Explosion, mCharacter->GetPosition2(), eventData))
    {
        if (glm::distance2(eventData.mPosition, mCharacter->GetPosition2()) > reactionDistance2) // too far away
            return false;

        return true;
    }
    return false;
}

bool AiCharacterController::ScanForGunshots()
{
    float reactionDistance2 = glm::pow(gGameParams.mAiReactOnGunshotsDistance, 2.0f);

    BroadcastEvent eventData;
    if (gBroadcastEvents.PeekClosestEvent(eBroadcastEvent_GunShot, mCharacter->GetPosition2(), eventData))
    {
        if (eventData.mCharacter == mCharacter) // hear own gunshots
            return false; 

        if (glm::distance2(eventData.mPosition, mCharacter->GetPosition2()) > reactionDistance2) // too far away
            return false;

        return true;
    }
    return false;
}

void AiCharacterController::UpdateFrame()
{
    // choose current activity
    if (mAiMode == ePedestrianAiMode_None)
    {
        if (IsControllerActive())
        {
            StartWandering();
            return;
        }

        mAiMode = ePedestrianAiMode_Disabled;
        return;
    }

    debug_assert(mCharacter);

    if (mCharacter->IsDead())
    {
        mAiMode = ePedestrianAiMode_Disabled;
        return;
    }

    if (mAiMode == ePedestrianAiMode_DrivingCar)
    {
        UpdateDrivingCar();
        return;
    }

    if (mCharacter->IsCarDriver())
    {
        StartDrivingCar();
        return;
    }

    if (!mCharacter->IsIdle())
        return;

    if (mAiMode == ePedestrianAiMode_Panic)
    {
        UpdatePanic();
        return;
    }
    else if (mCharacter->IsBurn())
    {
        StartPanic();
        return;
    }

    if (mAiMode == ePedestrianAiMode_Wandering)
    {
        UpdateWandering();
        return;
    }

    if (mAiMode == ePedestrianAiMode_FollowTarget)
    {
        UpdateFollowTarget();
        return;
    }
}

void AiCharacterController::UpdatePanic()
{
    if (ContinueWalkToWaypoint(mDefaultNearDistance))
        return;

    if (!ChooseWalkWaypoint(true) || !ContinueWalkToWaypoint(mDefaultNearDistance))
    {
        mAiMode = ePedestrianAiMode_Disabled; // disable ai
    }
}

void AiCharacterController::UpdateWandering()
{
    if (ScanForThreats())
    {
        StartPanic();
        return;
    }

    if (ContinueWalkToWaypoint(mDefaultNearDistance))
        return;

    bool canFollowHuman = HasAiFlags(ePedestrianAiFlags_FollowHumanCharacter);
    if (canFollowHuman)
    {
        if (TryFollowHumanCharacterNearby())
            return;
    }

    if (!ChooseWalkWaypoint(false) || !ContinueWalkToWaypoint(mDefaultNearDistance))
    {
        StartPanic();
    }
}

void AiCharacterController::StartPanic()
{
    mAiMode = ePedestrianAiMode_Panic;
    mFollowPedestrian.reset();

    mRunToTarget = true;

    mCharacter->mCtlState.Clear();
    if (!ChooseWalkWaypoint(true) || !ContinueWalkToWaypoint(mDefaultNearDistance))
    {
        mAiMode = ePedestrianAiMode_Disabled; // disable ai
    }
}

void AiCharacterController::StartWandering()
{
    mAiMode = ePedestrianAiMode_Wandering;
    mFollowPedestrian.reset();

    mCharacter->mCtlState.Clear();
    if (!ChooseWalkWaypoint(false) || !ContinueWalkToWaypoint(mDefaultNearDistance))
    {
        StartPanic();
    }
}

bool AiCharacterController::ChooseWalkWaypoint(bool isPanic)
{
    cxx::angle_t currHeading = mCharacter->mPhysicsBody->GetRotationAngle();

    // choose new block ir next order: forward, left, right, backward
    eMapDirection currentMapDirection = GetMapDirectionFromHeading(currHeading.mDegrees);
    eMapDirection moveDirs[] =
    {
        currentMapDirection,
        GetMapDirectionCCW(currentMapDirection),
        GetMapDirectionCW(currentMapDirection),
        GetMapDirectionOpposite(currentMapDirection)
    };

    glm::ivec3 currentLogPos = Convert::MetersToMapUnits(mCharacter->GetPosition());
    glm::ivec3 newWayPoint (0, 0, 0);
    for (eMapDirection curr: moveDirs)
    {
        glm::ivec3 moveBlockPos = currentLogPos + GetVectorFromMapDirection(curr);

        const MapBlockInfo* blockInfo = gGameMap.GetBlockInfo(moveBlockPos.x, moveBlockPos.z, moveBlockPos.y);

        eGroundType groundType = blockInfo->mGroundType;
        if (groundType == eGroundType_Pawement)
        {
            newWayPoint = moveBlockPos;
            break;
        }

        if (isPanic)
        {
            if ((groundType == eGroundType_Field) || (groundType == eGroundType_Road))
            {
                newWayPoint = moveBlockPos;
                break;
            }

            bool canSuicide = HasAiFlags(ePedestrianAiFlags_LemmingBehavior);
            if (canSuicide && (groundType == eGroundType_Air))
            {
                newWayPoint = moveBlockPos;
                break;
            }
        }
    }

    // nothing found
    if (newWayPoint == glm::ivec3(0, 0, 0))
    {
        return false;
    }

    // choose random point within block
    float randomSubPosx = gCarnageGame.mGameRand.generate_float(0.1f, 0.9f);
    float randomSubPosy = gCarnageGame.mGameRand.generate_float(0.1f, 0.9f);
    mDestinationPoint.x = Convert::MapUnitsToMeters(newWayPoint.x * 1.0f) + Convert::MapUnitsToMeters(randomSubPosx);
    mDestinationPoint.y = Convert::MapUnitsToMeters(newWayPoint.z * 1.0f) + Convert::MapUnitsToMeters(randomSubPosy);
    return true;
}

bool AiCharacterController::ContinueWalkToWaypoint(float distance)
{
    float tolerance2 = pow(gGameParams.mPedestrianBoundsSphereRadius, 2.0f);

    glm::vec2 currentPosition = mCharacter->mPhysicsBody->GetPosition2();
    if (glm::distance2(currentPosition, mDestinationPoint) <= tolerance2)
    {
        mCharacter->mCtlState.Clear();
        return false;
    }

    // setup sign direction
    glm::vec2 currentPos2 = mCharacter->mPhysicsBody->GetPosition2();
    glm::vec2 toTarget = glm::normalize(mDestinationPoint - currentPos2);
    mCharacter->mPhysicsBody->SetOrientation2(toTarget);

    // set control
    mCharacter->mCtlState.mWalkForward = true;
    mCharacter->mCtlState.mRun = mRunToTarget;
    return true;
}

void AiCharacterController::StartDrivingCar()
{
    mAiMode = ePedestrianAiMode_DrivingCar;
    mFollowPedestrian.reset();

    mCharacter->mCtlState.Clear();
    if (!ChooseDriveWaypoint() || !ContinueDriveToWaypoint())
    {
        StopDriving();
    }
}

void AiCharacterController::StopDriving()
{
    if (!mCharacter->IsCarDriver())
        return;

    mCharacter->mCtlState.Clear();

    float currentSpeed = mCharacter->mCurrentCar->mPhysicsBody->GetCurrentSpeed();
    if (currentSpeed > gGameParams.mVehicleSpeedPassengerCanEnter)
    {
        mCharacter->mCtlState.mAcceleration = -1.0f;
    }
    else
    {
        mCharacter->LeaveCar();
    }
}

void AiCharacterController::UpdateDrivingCar()
{
    if (!mCharacter->IsCarDriver())
    {
        StartPanic();
        return;
    }
    
    // leave heavily damaged car
    int currentDamage = mCharacter->mCurrentCar->GetCurrentDamage();
    if (currentDamage >= 80)
    {
        StopDriving();
        return;
    }

    if (ContinueDriveToWaypoint())
        return;

    if (!ChooseDriveWaypoint() || !ContinueDriveToWaypoint())
    {
        StopDriving();
    }
}

bool AiCharacterController::ChooseDriveWaypoint()
{
    // todo: implement
    return false;
}

bool AiCharacterController::ContinueDriveToWaypoint()
{
    // todo: implement
    return false;
}

void AiCharacterController::SetFollowPedestrian(Pedestrian* pedestrian)
{
    if (pedestrian == mCharacter || pedestrian == nullptr)
    {
        StartWandering();
        return;
    }
    mFollowPedestrian = pedestrian;
    StartFollowTarget();
}

void AiCharacterController::StartFollowTarget()
{
    if (!mFollowPedestrian)
    {
        debug_assert(false);
        StartWandering();
        return;
    }

    mCharacter->mCtlState.Clear();
    mAiMode = ePedestrianAiMode_FollowTarget;

    mDestinationPoint = mFollowPedestrian->mPhysicsBody->GetPosition2();
}

void AiCharacterController::UpdateFollowTarget()
{
    if (ScanForThreats())
    {
        StartPanic();
        return;
    }

    if (!mFollowPedestrian || mFollowPedestrian->IsDead())
    {
        StartWandering();
        return;
    }

    if (ContinueWalkToWaypoint(mFollowNearDistance))
        return;

    glm::vec2 characterPosition2 = mCharacter->mPhysicsBody->GetPosition2();
    glm::vec2 targetPosition2 = mFollowPedestrian->mPhysicsBody->GetPosition2();

    float distanceToTarget2 = glm::distance2(characterPosition2, targetPosition2);
    if (distanceToTarget2 < glm::pow(mFollowNearDistance, 2.0f))
    {
        mCharacter->mCtlState.Clear();
        return;
    }

    mRunToTarget = mFollowPedestrian->IsRunning() || (distanceToTarget2 > glm::pow(mFollowFarDistance, 2.0f));
    mDestinationPoint = targetPosition2 + glm::normalize(targetPosition2 - characterPosition2) * mFollowNearDistance;
    ContinueWalkToWaypoint(mFollowNearDistance);
}

void AiCharacterController::EnableAiFlags(ePedestrianAiFlags aiFlags)
{
    mAiFlags = (mAiFlags | aiFlags);
}

void AiCharacterController::DisableAiFlags(ePedestrianAiFlags aiFlags)
{
    mAiFlags = (mAiFlags & ~aiFlags);
}

bool AiCharacterController::HasAiFlags(ePedestrianAiFlags aiFlags) const
{
    return (mAiFlags & aiFlags) == aiFlags;
}

bool AiCharacterController::TryFollowHumanCharacterNearby()
{
    float maxSignDistance = Convert::MapUnitsToMeters(0.5f);

    float bestDistance2 = glm::pow(maxSignDistance, 2.0f);
    Pedestrian* bestHumanCharacter = nullptr;
    for (HumanPlayer* currentPlayer: gCarnageGame.mHumanPlayers)
    {
        if (currentPlayer == nullptr)
            continue;
        debug_assert(mCharacter != currentPlayer->mCharacter);
        if (!currentPlayer->mCharacter->IsStanding())
            continue;

        float currDistance2 = glm::distance2(currentPlayer->mCharacter->GetPosition2(), mCharacter->GetPosition2());
        if (currDistance2 > bestDistance2)
            continue;

        bestDistance2 = currDistance2;
        bestHumanCharacter = currentPlayer->mCharacter;
    }

    if (bestHumanCharacter)
    {
        SetFollowPedestrian(bestHumanCharacter);
        return true;
    }
    return false;
}

