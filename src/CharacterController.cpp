#include "stdafx.h"
#include "CharacterController.h"
#include "Pedestrian.h"
#include "Vehicle.h"

CharacterController::~CharacterController()
{
}

void CharacterController::UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
}

void CharacterController::ResetControlState(Pedestrian* pedestrian)
{
    // reset control actions
    for (int iaction = 0; iaction < ePedestrianAction_COUNT; ++iaction)
    {
        pedestrian->mCtlActions[iaction] = false;
    }
}

void CharacterController::HandleCharacterDeath(Pedestrian* pedestrian)
{

}

