#include "stdafx.h"
#include "HumanCharacterController.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Vehicle.h"

//////////////////////////////////////////////////////////////////////////

void InputActionsMapping::SetNull()
{
    mControllerType = eInputControllerType_None;

    mKeysInCarActions.clear();
    mKeysOnFootActions.clear();
    mGpButtonsInCarActions.clear();
    mGpButtonsOnFootActions.clear();
}

void InputActionsMapping::SetDefaults()
{
    mControllerType = eInputControllerType_Keyboard;

    // keys in car
    mKeysInCarActions[eKeycode_ENTER] = ePedestrianAction_LeaveCar;
    mKeysInCarActions[eKeycode_SPACE] = ePedestrianAction_HandBrake;
    mKeysInCarActions[eKeycode_UP] = ePedestrianAction_Accelerate;
    mKeysInCarActions[eKeycode_DOWN] = ePedestrianAction_Reverse;
    mKeysInCarActions[eKeycode_LEFT] = ePedestrianAction_SteerLeft;
    mKeysInCarActions[eKeycode_RIGHT] = ePedestrianAction_SteerRight;
    mKeysInCarActions[eKeycode_TAB] = ePedestrianAction_Horn;

    // keys on foot
    mKeysOnFootActions[eKeycode_LEFT] = ePedestrianAction_TurnLeft;
    mKeysOnFootActions[eKeycode_RIGHT] = ePedestrianAction_TurnRight;
    mKeysOnFootActions[eKeycode_SPACE] = ePedestrianAction_Jump;
    mKeysOnFootActions[eKeycode_DOWN] = ePedestrianAction_WalkBackward;
    mKeysOnFootActions[eKeycode_UP] = ePedestrianAction_Run;
    mKeysOnFootActions[eKeycode_LEFT_CTRL] = ePedestrianAction_Shoot;
    mKeysOnFootActions[eKeycode_X] = ePedestrianAction_NextWeapon;
    mKeysOnFootActions[eKeycode_Z] = ePedestrianAction_PrevWeapon;
    mKeysOnFootActions[eKeycode_ENTER] = ePedestrianAction_EnterCar;
    mKeysOnFootActions[eKeycode_F] = ePedestrianAction_EnterCarAsPassenger;
}

bool InputActionsMapping::SetFromConfig(cxx::config_node& configNode)
{
    const char* controller_type_str = configNode.get_child("controller_type").get_value_string();
    if (!cxx::parse_enum(controller_type_str, mControllerType))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Unknown controller type");
        return false;
    }

    auto ScanKeys = [](cxx::config_node& innode, std::map<eKeycode, ePedestrianAction>& inmap)
    {
        ePedestrianAction action = ePedestrianAction_null;
        eKeycode keycode = eKeycode_null;

        for (cxx::config_node currNode = innode.first_child(); currNode; currNode = currNode.next_sibling())
        {
            const char* keycode_str = currNode.get_value_string();
            if (!cxx::parse_enum(keycode_str, keycode))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Unknown keycode %s", keycode_str);
                continue;
            }

            const char* action_str = currNode.get_element_name();
            if (!cxx::parse_enum(action_str, action))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Unknown action %s", action_str);
                continue; 
            }
            inmap[keycode] = action;
        }
    };

    if (cxx::config_node keysNode = configNode.get_child("keys"))
    {
        if (cxx::config_node inCarNode = keysNode.get_child("in_car"))
        {
            ScanKeys(inCarNode, mKeysInCarActions);
        }

        if (cxx::config_node onFootNode = keysNode.get_child("on_foot"))
        {
            ScanKeys(onFootNode, mKeysOnFootActions);
        }
    }

    auto ScanGpButtons = [](cxx::config_node& innode, std::map<eGamepadButton, ePedestrianAction>& inmap)
    {
        ePedestrianAction action = ePedestrianAction_null;
        eGamepadButton gpButton = eGamepadButton_null;

        for (cxx::config_node currNode = innode.first_child(); currNode; currNode = currNode.next_sibling())
        {
            const char* gpbutton_str = currNode.get_value_string();
            if (!cxx::parse_enum(gpbutton_str, gpButton))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Unknown gamepad button %s", gpbutton_str);
                continue;
            }

            const char* action_str = currNode.get_element_name();
            if (!cxx::parse_enum(action_str, action))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Unknown action %s", action_str);
                continue; 
            }
            inmap[gpButton] = action;
        }
    };

    if (cxx::config_node gpNode = configNode.get_child("gamepad"))
    {
        if (cxx::config_node inCarNode = gpNode.get_child("in_car"))
        {
            ScanGpButtons(inCarNode, mGpButtonsInCarActions);
        }

        if (cxx::config_node onFootNode = gpNode.get_child("on_foot"))
        {
            ScanGpButtons(onFootNode, mGpButtonsOnFootActions);
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

void HumanCharacterController::UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    debug_assert(pedestrian == mCharacter);
}

void HumanCharacterController::InputEvent(KeyInputEvent& inputEvent)
{
    if (mCharacter == nullptr || inputEvent.mKeycode == eKeycode_null)
        return;

    if (mInputs.mControllerType != eInputControllerType_Keyboard)
    {
        return;
    }

    ePedestrianAction action = ePedestrianAction_null;
    if (mCharacter->IsCarPassenger())
    {
        auto iaction = mInputs.mKeysInCarActions.find(inputEvent.mKeycode);
        if (iaction != mInputs.mKeysInCarActions.end())
        {
            action = iaction->second;
        }
    }
    else
    {
        auto iaction = mInputs.mKeysOnFootActions.find(inputEvent.mKeycode);
        if (iaction != mInputs.mKeysOnFootActions.end())
        {
            action = iaction->second;
        }
    }
    if (action == ePedestrianAction_null)
        return;

    if (HandleInputAction(action, inputEvent.mPressed))
    {
        inputEvent.SetConsumed();
        return;
    }
}

void HumanCharacterController::InputEvent(GamepadInputEvent& inputEvent)
{
    if (mCharacter == nullptr || inputEvent.mButton == eGamepadButton_null)
        return;

    static const eInputControllerType gamepadControllers[MAX_GAMEPADS] =
    {
        eInputControllerType_Gamepad1,
        eInputControllerType_Gamepad2,
        eInputControllerType_Gamepad3,
        eInputControllerType_Gamepad4
    };

    if (inputEvent.mGamepad < MAX_GAMEPADS)
    {
        eInputControllerType controllerType = gamepadControllers[inputEvent.mGamepad];
        if (controllerType != mInputs.mControllerType)
            return;

        ePedestrianAction action = ePedestrianAction_null;
        if (mCharacter->IsCarPassenger())
        {
            auto iaction = mInputs.mGpButtonsInCarActions.find(inputEvent.mButton);
            if (iaction != mInputs.mGpButtonsInCarActions.end())
            {
                action = iaction->second;
            }
        }
        else
        {
            auto iaction = mInputs.mGpButtonsOnFootActions.find(inputEvent.mButton);
            if (iaction != mInputs.mGpButtonsOnFootActions.end())
            {
                action = iaction->second;
            }
        }
        if (action == ePedestrianAction_null)
            return;

        if (HandleInputAction(action, inputEvent.mPressed))
        {
            inputEvent.SetConsumed();
            return;
        }
    }
}

bool HumanCharacterController::HandleInputAction(ePedestrianAction action, bool isActivated)
{
    debug_assert(mCharacter);
    
    switch (action)
    {
        case ePedestrianAction_SteerLeft:
        case ePedestrianAction_TurnLeft:
        case ePedestrianAction_SteerRight:
        case ePedestrianAction_TurnRight:
        case ePedestrianAction_Run:
        case ePedestrianAction_Accelerate:
        case ePedestrianAction_Reverse:
        case ePedestrianAction_WalkBackward:
        case ePedestrianAction_WalkForward:
        case ePedestrianAction_Jump:
        case ePedestrianAction_HandBrake:
        case ePedestrianAction_Shoot:
            mCharacter->mCtlActions[action] = isActivated;
        break;

        case ePedestrianAction_Horn:
            if (mCharacter->IsCarPassenger())
            {
                mCharacter->mCtlActions[ePedestrianAction_Horn] = isActivated;
                if (mCharacter->mCurrentCar->HasEmergencyLightsAnimation())
                {
                    mCharacter->mCurrentCar->EnableEmergencyLights(isActivated);
                }
            }
        break;

        case ePedestrianAction_NextWeapon:
            if (isActivated)
            {
                SwitchNextWeapon();
            }
        break;

        case ePedestrianAction_PrevWeapon:
            if (isActivated)
            {
                SwitchPrevWeapon();
            }
        break;

        case ePedestrianAction_EnterCar:
        case ePedestrianAction_LeaveCar:
        case ePedestrianAction_EnterCarAsPassenger:
            if (isActivated)
            {
                EnterOrExitCar(action == ePedestrianAction_EnterCarAsPassenger);
            }
        break;

        default:    
            debug_assert(false);
        return false;
    }

    return true;
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
    int nextWeaponIndex = (mCharacter->mCurrentWeapon + 1) % eWeaponType_COUNT;
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        if (mCharacter->mWeaponsAmmo[nextWeaponIndex] != 0)
        {
            mCharacter->ChangeWeapon((eWeaponType) nextWeaponIndex);
            return;
        }
        nextWeaponIndex = (nextWeaponIndex + 1) % eWeaponType_COUNT;
    }
}

void HumanCharacterController::SwitchPrevWeapon()
{
    int nextWeaponIndex = mCharacter->mCurrentWeapon == 0 ? (eWeaponType_COUNT - 1) : (mCharacter->mCurrentWeapon - 1);
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        if (mCharacter->mWeaponsAmmo[nextWeaponIndex] != 0)
        {
            mCharacter->ChangeWeapon((eWeaponType) nextWeaponIndex);
            return;
        }
        nextWeaponIndex = nextWeaponIndex == 0 ? (eWeaponType_COUNT - 1) : (nextWeaponIndex - 1);
    }
}

void HumanCharacterController::EnterOrExitCar(bool alternative)
{
    if (mCharacter->IsCarPassenger())
    {
        mCharacter->LeaveCar();
        return;
    }

    PhysicsQueryResult queryResult;

    glm::vec3 pos = mCharacter->mPhysicsComponent->GetPosition();
    glm::vec2 posA { pos.x, pos.z };
    glm::vec2 posB = posA + (mCharacter->mPhysicsComponent->GetSignVector() * gGameRules.mPedestrianSpotTheCarDistance);

    gPhysics.QueryObjects(posA, posB, queryResult);

    // process all cars
    for (int icar = 0; icar < queryResult.mCarsCount; ++icar)
    {
        Vehicle* currCar = queryResult.mCarsList[icar]->mReferenceCar;

        mCharacter->TakeSeatInCar(currCar, alternative ? eCarSeat_Passenger : eCarSeat_Driver);
        return;
    }
}
