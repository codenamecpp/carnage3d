#include "stdafx.h"
#include "CharacterController.h"
#include "Pedestrian.h"
#include "Vehicle.h"

CharacterController::~CharacterController()
{
}

void CharacterController::UpdateFrame(Pedestrian* pedestrian)
{
    // do nothing
}

void CharacterController::ResetControlState(Pedestrian* pedestrian)
{
    pedestrian->mCtlState.Clear();
}

void CharacterController::HandleCharacterDeath(Pedestrian* pedestrian)
{
    // do nothing
}