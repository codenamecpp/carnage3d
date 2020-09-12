#include "stdafx.h"
#include "HumanPlayer.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Vehicle.h"
#include "GameMapManager.h"
#include "CarnageGame.h"
#include "TimeManager.h"

HumanPlayer::HumanPlayer()
{
}

void HumanPlayer::UpdateFrame()
{
    mPlayerView.UpdateFrame();

    if (mCharacter->IsDead())
    {
        // detect death
        if (mRespawnTime == 0.0f)
        {
            mRespawnTime = gGameParams.mGamePlayerRespawnTime;

            int playerIndex = gCarnageGame.GetHumanPlayerIndex(this);
            gConsole.LogMessage(eLogMessage_Info, "Player %d died (%s)", (playerIndex + 1), cxx::enum_to_string(mCharacter->mDeathReason));
            // todo: cleanup this mess
            mPlayerView.mHUD.ShowBigFontMessage(eHUDBigFontMessage_Wasted);
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

void HumanPlayer::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mPlayerView.InputEvent(inputEvent);
}

void HumanPlayer::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mPlayerView.InputEvent(inputEvent);
}

void HumanPlayer::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mPlayerView.InputEvent(inputEvent);
}

void HumanPlayer::InputEvent(KeyInputEvent& inputEvent)
{
    debug_assert(mCharacter);

    mPlayerView.InputEvent(inputEvent);

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

void HumanPlayer::InputEvent(GamepadInputEvent& inputEvent)
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

void HumanPlayer::InputEventLost()
{
    mPlayerView.InputEventLost();

    if (mCharacter)
    {
        // reset actions
        mCharacter->mCtlState.Clear();
    }

    mUpdateInputs = false;
}

void HumanPlayer::ProcessInputAction(eInputAction action, bool isActivated)
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

void HumanPlayer::SetCharacter(Pedestrian* character)
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

void HumanPlayer::SwitchNextWeapon()
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

void HumanPlayer::SwitchPrevWeapon()
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

void HumanPlayer::EnterOrExitCar(bool alternative)
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

void HumanPlayer::Respawn()
{
    mRespawnTime = 0.0f;

    // todo : exit from car

    mCharacter->Spawn(mSpawnPosition, mCharacter->mPhysicsBody->GetRotationAngle());
}

void HumanPlayer::ProcessRepetitiveActions()
{
    if (!mUpdateInputs)
        return;

    debug_assert(mCharacter);
    PedestrianCtlState& ctlState = mCharacter->mCtlState;
    ctlState.Clear();

    // update in car
    if (mCharacter->IsCarPassenger())
    {
        ctlState.mSteerDirection = 0.0f;
        if (GetActionState(eInputAction_SteerRight))
        {
            ctlState.mSteerDirection += 1.0f;
        }
        if (GetActionState(eInputAction_SteerLeft))
        {
            ctlState.mSteerDirection -= 1.0f;
        }

        ctlState.mAcceleration = 0.0f;
        if (GetActionState(eInputAction_Accelerate))
        {
            ctlState.mAcceleration += 1.0f;
        }
        if (GetActionState(eInputAction_Reverse))
        {
            ctlState.mAcceleration -= 1.0f;
        }

        ctlState.mHandBrake = GetActionState(eInputAction_HandBrake);
    }
    // update on foot
    else
    {
        ctlState.mTurnLeft = GetActionState(eInputAction_TurnLeft);
        ctlState.mTurnRight = GetActionState(eInputAction_TurnRight);
        ctlState.mRun = GetActionState(eInputAction_Run);
        ctlState.mWalkBackward = GetActionState(eInputAction_WalkBackward);
        ctlState.mWalkForward = GetActionState(eInputAction_WalkForward);
        ctlState.mJump = GetActionState(eInputAction_Jump);
        ctlState.mShoot = GetActionState(eInputAction_Shoot);
    }
}

bool HumanPlayer::GetActionState(eInputAction action) const
{
    const auto& mapping = mActionsMapping.mActionToKeys[action];
    if (mapping.mKeycode != eKeycode_null)
    {
        if (gInputs.GetKeyState(mapping.mKeycode))
            return true;
    }
    if (mapping.mGpButton != eGamepadButton_null)
    {
        if (gInputs.GetGamepadButtonState(mActionsMapping.mGamepadID, mapping.mGpButton))
            return true;
    }
    return false;
}

void HumanPlayer::DeactivateConstroller()
{
    // do nothing
}

bool HumanPlayer::IsHumanPlayer() const
{
    return true;
}

void HumanPlayer::OnCharacterStartCarDrive()
{
    Vehicle* currentCar = mCharacter->mCurrentCar;
    if (currentCar == nullptr)
    {
        debug_assert(false);
        return;
    }
    eVehicleModel carModel = currentCar->mCarInfo->mModelID;
    mPlayerView.mHUD.ShowCarNameMessage(carModel);
}

