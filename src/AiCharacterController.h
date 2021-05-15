#pragma once

#include "CharacterController.h"
#include "Pedestrian.h"

//////////////////////////////////////////////////////////////////////////

enum ePedestrianAiMode
{
    ePedestrianAiMode_None,
    ePedestrianAiMode_Disabled,
    ePedestrianAiMode_Wandering,
    ePedestrianAiMode_Panic,
    ePedestrianAiMode_DrivingCar,
    ePedestrianAiMode_FollowTarget,
};

enum ePedestrianAiState
{
    ePedestrianAiState_Idle,
    ePedestrianAiState_RunToLocation,
    ePedestrianAiState_WalkToLocation,
};

enum PedestrianAiFlags
{
    PedestrianAiFlags_None = 0,

    PedestrianAiFlags_LemmingBehavior = BIT(0), // can suicide
    PedestrianAiFlags_FollowHumanCharacter = BIT(1),
};

decl_enum_as_flags(PedestrianAiFlags);

//////////////////////////////////////////////////////////////////////////

// defines ai character controller
class AiCharacterController final: public CharacterController
{
public:
    AiCharacterController(Pedestrian* character);

    // process controller logic
    void DebugDraw(DebugRenderer& debugRender) override;
    void OnCharacterUpdateFrame() override;

    void ChangeAiFlags(PedestrianAiFlags enableFlags, PedestrianAiFlags disableFlags);
    bool HasAiFlags(PedestrianAiFlags aiFlags) const;

    // objectives
    void FollowPedestrian(Pedestrian* pedestrian);

private:
    void UpdatePanic();
    void UpdateWandering();
    void UpdateDrivingCar();
    void UpdateFollowTarget();

    void StartPanic();
    void StartWandering();
    void StartDrivingCar();
    void StartFollowTarget();

    bool TryFollowHumanCharacterNearby();

    // walk/run
    bool ChooseWalkWaypoint(bool isPanic);
    bool ContinueWalkToWaypoint(float distance);

    // drive
    bool ChooseDriveWaypoint();
    bool ContinueDriveToWaypoint();
    void StopDriving();

    // utility
    bool ScanForThreats();
    bool ScanForGunshots();
    bool ScanForExplosions();

private:
    ePedestrianAiMode mAiMode = ePedestrianAiMode_None;
    ePedestrianAiState mAiState = ePedestrianAiState_Idle;

    glm::vec2 mDestinationPoint;
    float mDefaultNearDistance;
    
    PedestrianAiFlags mAiFlags = PedestrianAiFlags_None;

    PedestrianHandle mFollowPedestrian;
    float mFollowNearDistance;
    float mFollowFarDistance;

    bool mRunToTarget = false;
};