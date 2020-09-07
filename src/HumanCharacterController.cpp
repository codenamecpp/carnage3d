#include "stdafx.h"
#include "HumanCharacterController.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Vehicle.h"
#include "GameMapManager.h"
#include "CarnageGame.h"
#include "TimeManager.h"

static const float PlayerCharacterRespawnTime = 10.0f;

void HumanCharacterController::UpdateFrame()
{
    if (mCharacter->IsDead())
    {
        // detect death
        if (mRespawnTime == 0.0f)
        {
            mRespawnTime = PlayerCharacterRespawnTime;

            int playerIndex = 1 + gCarnageGame.GetPlayerIndex(this);
            gConsole.LogMessage(eLogMessage_Info, "Player %d died (%s)", playerIndex, cxx::enum_to_string(mCharacter->mDeathReason));
        }

        float deltaTime = gTimeManager.mGameFrameDelta;
        mRespawnTime -= deltaTime;
        if (mRespawnTime < 0)
        {
            Respawn();
        }
    }

    ProcessRepetitiveActions();
}

void HumanCharacterController::InputEvent(KeyInputEvent& inputEvent)
{
    debug_assert(mCharacter);

    eInputActionsGroup actionGroup = mCharacter->IsCarPassenger() ? eInputActionsGroup_InCar : eInputActionsGroup_OnFoot;
    eInputAction action = mActionsMapping.GetAction(actionGroup, inputEvent.mKeycode);
    if (action == eInputAction_null)
    {
        // try commons
        action = mActionsMapping.GetAction(eInputActionsGroup_Common, inputEvent.mKeycode);
    }

    if (action != eInputAction_null)
    {
        ProcessInputAction(action, inputEvent.mPressed);
        inputEvent.SetConsumed();
    }
}

void HumanCharacterController::InputEvent(GamepadInputEvent& inputEvent)
{
    debug_assert(mCharacter);

    if (inputEvent.mGamepad != mActionsMapping.mGamepadID)
        return;

    eInputActionsGroup actionGroup = mCharacter->IsCarPassenger() ? eInputActionsGroup_InCar : eInputActionsGroup_OnFoot;
    eInputAction action = mActionsMapping.GetAction(actionGroup, inputEvent.mButton);
    if (action == eInputAction_null)
    {
        // try commons
        action = mActionsMapping.GetAction(eInputActionsGroup_Common, inputEvent.mButton);
    }

    if (action != eInputAction_null)
    {
        ProcessInputAction(action, inputEvent.mPressed);
        inputEvent.SetConsumed();
    }
}

void HumanCharacterController::InputEventLost()
{
    if (mCharacter)
    {
        // reset actions
        mCharacter->mCtlState.Clear();
    }

    mUpdateInputs = false;
}

void HumanCharacterController::ProcessInputAction(eInputAction action, bool isActivated)
{    
    PedestrianCtlState& ctlState = mCharacter->mCtlState;
    switch (action)
    {
        case eInputAction_SteerLeft:
        case eInputAction_TurnLeft:
        case eInputAction_SteerRight:
        case eInputAction_TurnRight:
        case eInputAction_Run:
        case eInputAction_Accelerate:
        case eInputAction_Reverse:
        case eInputAction_WalkBackward:
        case eInputAction_WalkForward:
        case eInputAction_Jump:
        case eInputAction_HandBrake:
        case eInputAction_Shoot:
            // those are updating in ProcessRepetitiveActions
        break;

        case eInputAction_Horn:
            if (mCharacter->IsCarPassenger())
            {
                ctlState.mHorn = isActivated;
                if (mCharacter->mCurrentCar->HasEmergencyLightsAnimation())
                {
                    mCharacter->mCurrentCar->EnableEmergencyLights(isActivated);
                }
            }
        break;

        case eInputAction_NextWeapon:
            if (isActivated)
            {
                SwitchNextWeapon();
            }
        break;

        case eInputAction_PrevWeapon:
            if (isActivated)
            {
                SwitchPrevWeapon();
            }
        break;

        case eInputAction_EnterCar:
        case eInputAction_LeaveCar:
        case eInputAction_EnterCarAsPassenger:
            if (isActivated)
            {
                EnterOrExitCar(action == eInputAction_EnterCarAsPassenger);
            }
        break;

        default:
            debug_assert(false);
        break;
    }

    mUpdateInputs = true;
}

void HumanCharacterController::SetCharacter(Pedestrian* character)
{
    if (mCharacter)
    {
        debug_assert(mCharacter->mController == this);
        mCharacter->mController = nullptr;
    }
    mCharacter = character;
    if (mCharacter)
    {
        mCharacter->mController = this;
    }
}

void HumanCharacterController::SwitchNextWeapon()
{
    int nextWeaponIndex = (mCharacter->mCurrentWeapon + 1) % eWeapon_COUNT;
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        if (mCharacter->mWeaponsAmmo[nextWeaponIndex] != 0)
        {
            mCharacter->ChangeWeapon((eWeaponID) nextWeaponIndex);
            return;
        }
        nextWeaponIndex = (nextWeaponIndex + 1) % eWeapon_COUNT;
    }
}

void HumanCharacterController::SwitchPrevWeapon()
{
    int nextWeaponIndex = mCharacter->mCurrentWeapon == 0 ? (eWeapon_COUNT - 1) : (mCharacter->mCurrentWeapon - 1);
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        if (mCharacter->mWeaponsAmmo[nextWeaponIndex] != 0)
        {
            mCharacter->ChangeWeapon((eWeaponID) nextWeaponIndex);
            return;
        }
        nextWeaponIndex = nextWeaponIndex == 0 ? (eWeapon_COUNT - 1) : (nextWeaponIndex - 1);
    }
}

void HumanCharacterController::EnterOrExitCar(bool alternative)
{
    if (mCharacter->IsCarPassenger())
    {
        mCharacter->LeaveCar();
        return;
    }

    PhysicsLinecastResult linecastResult;

    glm::vec3 pos = mCharacter->mPhysicsBody->GetPosition();
    glm::vec2 posA { pos.x, pos.z };
    glm::vec2 posB = posA + (mCharacter->mPhysicsBody->GetSignVector() * gGameParams.mPedestrianSpotTheCarDistance);

    gPhysics.QueryObjectsLinecast(posA, posB, linecastResult);

    // process all cars
    for (int icar = 0; icar < linecastResult.mHitsCount; ++icar)
    {
        CarPhysicsBody* carBody = linecastResult.mHits[icar].mCarComponent;
        if (carBody == nullptr)
            continue;

        eCarSeat carSeat = alternative ? eCarSeat_Passenger : eCarSeat_Driver;
        if (carBody->mReferenceCar->IsSeatPresent(carSeat))
        {
            mCharacter->EnterCar(carBody->mReferenceCar, carSeat);
        }
        return;
    }
}

void HumanCharacterController::Respawn()
{
    mRespawnTime = 0.0f;

    // todo : exit from car

    mCharacter->Spawn(mSpawnPosition, mCharacter->mPhysicsBody->GetRotationAngle());
}

void HumanCharacterController::ProcessRepetitiveActions()
{
    if (!mUpdateInputs)
        return;

    debug_assert(mCharacter);
    PedestrianCtlState& ctlState = mCharacter->mCtlState;
    ctlState.Clear();

    // update in car
    if (mCharacter->IsCarPassenger())
    {
        SyncActionState(eInputAction_SteerLeft, ctlState.mSteerLeft);
        SyncActionState(eInputAction_SteerRight, ctlState.mSteerRight);
        SyncActionState(eInputAction_Accelerate, ctlState.mAccelerate);
        SyncActionState(eInputAction_Reverse, ctlState.mReverse);
        SyncActionState(eInputAction_HandBrake, ctlState.mHandBrake);
    }
    // update on foot
    else
    {
        SyncActionState(eInputAction_TurnLeft, ctlState.mTurnLeft);
        SyncActionState(eInputAction_TurnRight, ctlState.mTurnRight);
        SyncActionState(eInputAction_Run, ctlState.mRun);
        SyncActionState(eInputAction_WalkBackward, ctlState.mWalkBackward);
        SyncActionState(eInputAction_WalkForward, ctlState.mWalkForward);
        SyncActionState(eInputAction_Jump, ctlState.mJump);
        SyncActionState(eInputAction_Shoot, ctlState.mShoot);
    }
}

void HumanCharacterController::SyncActionState(eInputAction action, bool& stateFlag) const
{
    stateFlag = false;

    const auto& mapping = mActionsMapping.mActionToKeys[action];
    if (mapping.mKeycode != eKeycode_null)
    {
        if (gInputs.GetKeyState(mapping.mKeycode))
        {
            stateFlag = true;
        }
    }
    if (mapping.mGpButton != eGamepadButton_null)
    {
        if (gInputs.GetGamepadButtonState(mActionsMapping.mGamepadID, mapping.mGpButton))
        {
            stateFlag = true;
        }
    }
}
