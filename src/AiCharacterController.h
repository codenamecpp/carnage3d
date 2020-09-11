#pragma once

#include "CharacterController.h"
#include "Pedestrian.h"

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

// defines ai character controller
class AiCharacterController final: public CharacterController
{
public:
    AiCharacterController(Pedestrian* character);

    // process controller logic
    void UpdateFrame() override;
    void DebugDraw(DebugRenderer& debugRender) override;

    void SetLemmingBehavior(bool canSuicide);
    void SetFollowPedestrian(Pedestrian* pedestrian);

private:
    void UpdatePanic();
    void UpdateWandering();
    void UpdateDrivingCar();
    void UpdateFollowTarget();

    void StartPanic();
    void StartWandering();
    void StartDrivingCar();
    void StartFollowTarget();

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
    bool mIsLemmingBehavior = true;

    PedestrianHandle mFollowPedestrian;
    float mFollowNearDistance;
    float mFollowFarDistance;

    bool mRunToTarget = false;
};