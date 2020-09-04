#pragma once

#include "CharacterController.h"

enum ePedestrianAiMode
{
    ePedestrianAiMode_None,
    ePedestrianAiMode_Wandering,
    ePedestrianAiMode_Panic,
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

    void SetCanSuicideInPanic(bool canSuicide);

private:
    void UpdatePanic();
    void UpdateWandering();

    void StartPanic();
    void StartWandering();

    bool ChooseRandomWayPoint(bool isPanic);
    bool ContinueMoveToPoint(bool isPanic);

private:
    ePedestrianAiMode mAiMode = ePedestrianAiMode_None;
    ePedestrianAiState mAiState = ePedestrianAiState_Idle;

    glm::vec2 mDestinationPoint;

    bool mCanSuicideInPanic = true;
};