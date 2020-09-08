#pragma once

#include "CharacterController.h"

enum ePedestrianAiMode
{
    ePedestrianAiMode_None,
    ePedestrianAiMode_Wandering,
    ePedestrianAiMode_Panic,
    ePedestrianAiMode_DrivingCar,
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

private:
    void UpdatePanic();
    void UpdateWandering();
    void UpdateDrivingCar();

    void StartPanic();
    void StartWandering();
    void StartDrivingCar();

    // walk/run
    bool ChooseWalkWaypoint(bool isPanic);
    bool ContinueWalkToWaypoint(bool isPanic);

    // drive
    bool ChooseDriveWaypoint();
    bool ContinueDriveToWaypoint();
    void StopDriving();

private:
    ePedestrianAiMode mAiMode = ePedestrianAiMode_None;
    ePedestrianAiState mAiState = ePedestrianAiState_Idle;

    glm::vec2 mDestinationPoint;
    bool mIsLemmingBehavior = true;
};