#include "stdafx.h"
#include "AiCharacterController.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "CarnageGame.h"
#include "DebugRenderer.h"

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

    if (mCharacter)
    {
        debug_assert(mCharacter->mController == nullptr);
        mCharacter->mController = this;

        StartWandering();
    }
}

void AiCharacterController::UpdateFrame()
{
    if (mAiMode == ePedestrianAiMode_None)
        return;

    debug_assert(mCharacter);

    if (mCharacter->IsDead())
    {
        mAiMode = ePedestrianAiMode_None;
        return;
    }

    if ((mAiMode == ePedestrianAiMode_DrivingCar) && mCharacter->IsCarDriver())
    {
        UpdateDrivingCar();
        return;
    }

    if (mAiMode == ePedestrianAiMode_DrivingCar)
    {
        StartPanic();
        return;
    }

    if (mCharacter->IsCarDriver())
    {
        StartDrivingCar();
        return;
    }

    if (!mCharacter->IsIdle())
        return;

    if (mAiMode == ePedestrianAiMode_Wandering)
    {
        UpdateWandering();
        return;
    }

    if (mAiMode == ePedestrianAiMode_Panic)
    {
        UpdatePanic();
        return;
    }
}

void AiCharacterController::UpdatePanic()
{
    if (ContinueMoveToPoint(true))
        return;

    if (!ChooseRandomWayPoint(true) || !ContinueMoveToPoint(true))
    {
        mAiMode = ePedestrianAiMode_None; // disable ai
    }
}

void AiCharacterController::UpdateWandering()
{
    if (mCharacter->IsBurn())
    {
        StartPanic();
        return;
    }

    if (ContinueMoveToPoint(false))
        return;

    if (!ChooseRandomWayPoint(false) || !ContinueMoveToPoint(false))
    {
        StartPanic();
    }
}

void AiCharacterController::StartPanic()
{
    mAiMode = ePedestrianAiMode_Panic;

    mCharacter->mCtlState.Clear();
    if (!ChooseRandomWayPoint(true) || !ContinueMoveToPoint(true))
    {
        mAiMode = ePedestrianAiMode_None; // disable ai
    }
}

void AiCharacterController::StartWandering()
{
    mAiMode = ePedestrianAiMode_Wandering;

    mCharacter->mCtlState.Clear();
    if (!ChooseRandomWayPoint(false) || !ContinueMoveToPoint(false))
    {
        StartPanic();
    }
}

bool AiCharacterController::ChooseRandomWayPoint(bool isPanic)
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

    glm::ivec3 currentLogPos = mCharacter->GetLogicalPosition();
    glm::ivec3 newWayPoint (0, 0, 0);
    for (eMapDirection curr: moveDirs)
    {
        glm::ivec3 moveBlockPos = currentLogPos + GetVectorFromMapDirection(curr);

        const MapBlockInfo* blockInfo = gGameMap.GetBlockClamp(moveBlockPos.x, moveBlockPos.z, moveBlockPos.y);

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

            if (mCanSuicideInPanic && (groundType == eGroundType_Air))
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
    float randomSubPosx = glm::clamp(gCarnageGame.mGameRand.generate_float(), 0.1f, 0.9f);
    float randomSubPosy = glm::clamp(gCarnageGame.mGameRand.generate_float(), 0.1f, 0.9f);
    mDestinationPoint.x = Convert::MapUnitsToMeters(newWayPoint.x * 1.0f) + Convert::MapUnitsToMeters(randomSubPosx);
    mDestinationPoint.y = Convert::MapUnitsToMeters(newWayPoint.z * 1.0f) + Convert::MapUnitsToMeters(randomSubPosy);
    return true;
}

bool AiCharacterController::ContinueMoveToPoint(bool isPanic)
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
    mCharacter->mCtlState.mRun = isPanic;
    return true;
}

void AiCharacterController::DebugDraw(DebugRenderer& debugRender)
{
    if (mAiMode == ePedestrianAiMode_Panic || mAiMode == ePedestrianAiMode_Wandering)
    {
        glm::vec3 currpos = mCharacter->mPhysicsBody->GetPosition();
        glm::vec3 destpos (mDestinationPoint.x, currpos.y, mDestinationPoint.y);

        debugRender.DrawLine(currpos, destpos, Color32_Red, false);
    }
}

void AiCharacterController::SetCanSuicideInPanic(bool canSuicide)
{
    mCanSuicideInPanic = canSuicide;
}

void AiCharacterController::StartDrivingCar()
{
    mAiMode = ePedestrianAiMode_DrivingCar;

    mCharacter->mCtlState.Clear();
}

void AiCharacterController::UpdateDrivingCar()
{
    // todo: temporary implementation

    cxx::angle_t currHeading = mCharacter->mPhysicsBody->GetRotationAngle();
    eMapDirection currentMapDirection = GetMapDirectionFromHeading(currHeading.mDegrees);

    eMapDirection moveDirs[] =
    {
        currentMapDirection,
        GetMapDirectionCCW(currentMapDirection),
        GetMapDirectionCW(currentMapDirection),
    };

    glm::ivec3 currentLogPos = mCharacter->GetLogicalPosition();
    glm::ivec3 newWayPoint (0, 0, 0);
    for (eMapDirection curr: moveDirs)
    {
        glm::ivec3 moveBlockPos = currentLogPos + GetVectorFromMapDirection(curr);

        const MapBlockInfo* blockInfo = gGameMap.GetBlockClamp(moveBlockPos.x, moveBlockPos.z, moveBlockPos.y);
        if (blockInfo->mGroundType != eGroundType_Road)
            continue;

        if (blockInfo->mUpDirection && curr != eMapDirection_N)
            continue;

        if (blockInfo->mLeftDirection && curr != eMapDirection_W)
            continue;

        if (blockInfo->mRightDirection && curr != eMapDirection_E)
            continue;

        if (blockInfo->mDownDirection && curr != eMapDirection_S)
            continue;

        mCharacter->mCtlState.mAccelerate = true;
        mCharacter->mCtlState.mReverse = false;
        mCharacter->mCtlState.mSteerLeft = false;
        mCharacter->mCtlState.mSteerRight = false;

        if (curr == currentMapDirection)
            continue;

        if (curr == GetMapDirectionCCW(currentMapDirection))
        {
            mCharacter->mCtlState.mSteerLeft = true;
            continue;
        }
        if (curr == GetMapDirectionCW(currentMapDirection))
        {
            mCharacter->mCtlState.mSteerRight = true;
            continue;
        }
        mCharacter->mCtlState.mReverse = true;
        mCharacter->mCtlState.mAccelerate = false;
    }
}
