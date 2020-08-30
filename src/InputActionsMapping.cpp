#include "stdafx.h"
#include "InputActionsMapping.h"

struct DefaultActionMapping
{
public:
    DefaultActionMapping(eInputActionsGroup group, eInputAction action, eKeycode keycode, eGamepadButton gpButton = eGamepadButton_null)
        : mActionGroup(group)
        , mAction(action)
        , mKeycode(keycode)
        , mGpButton(gpButton)
    {
    }
public:
    eInputActionsGroup mActionGroup;
    eInputAction mAction;
    eKeycode mKeycode;
    eGamepadButton mGpButton;
};

static const DefaultActionMapping gDefaultActionsMapping[] = 
{
    // common
    {eInputActionsGroup_Common, eInputAction_NextWeapon,            eKeycode_X},
    {eInputActionsGroup_Common, eInputAction_PrevWeapon,            eKeycode_Z},

    // in car
    {eInputActionsGroup_InCar,  eInputAction_LeaveCar,              eKeycode_ENTER},
    {eInputActionsGroup_InCar,  eInputAction_HandBrake,             eKeycode_SPACE},
    {eInputActionsGroup_InCar,  eInputAction_Accelerate,            eKeycode_UP},
    {eInputActionsGroup_InCar,  eInputAction_Reverse,               eKeycode_DOWN},
    {eInputActionsGroup_InCar,  eInputAction_SteerLeft,             eKeycode_LEFT},
    {eInputActionsGroup_InCar,  eInputAction_SteerRight,            eKeycode_RIGHT},
    {eInputActionsGroup_InCar,  eInputAction_Horn,                  eKeycode_TAB},

    // on foot
    {eInputActionsGroup_OnFoot, eInputAction_TurnLeft,              eKeycode_LEFT},
    {eInputActionsGroup_OnFoot, eInputAction_TurnRight,             eKeycode_RIGHT},
    {eInputActionsGroup_OnFoot, eInputAction_Jump,                  eKeycode_SPACE},
    {eInputActionsGroup_OnFoot, eInputAction_WalkBackward,          eKeycode_DOWN},
    {eInputActionsGroup_OnFoot, eInputAction_Run,                   eKeycode_UP},
    {eInputActionsGroup_OnFoot, eInputAction_Shoot,                 eKeycode_LEFT_CTRL},
    {eInputActionsGroup_OnFoot, eInputAction_EnterCar,              eKeycode_ENTER},
    {eInputActionsGroup_OnFoot, eInputAction_EnterCarAsPassenger,   eKeycode_F},
};

InputActionsMapping::InputActionsMapping()
{
    Clear();
}

void InputActionsMapping::Clear()
{
    mGamepadID = eGamepadID_Gamepad1;

    ::memset(mKeycodeActions, 0, sizeof(mKeycodeActions));
    ::memset(mGpButtonActions, 0, sizeof(mGpButtonActions));
    ::memset(mActionToKeys, 0, sizeof(mActionToKeys));
}

void InputActionsMapping::SetDefaults()
{
    mGamepadID = eGamepadID_Gamepad1;

    for (const DefaultActionMapping& curr: gDefaultActionsMapping)
    {
        if (curr.mKeycode != eKeycode_null)
        {
            mKeycodeActions[curr.mKeycode].mActions[curr.mActionGroup] = curr.mAction;
        }

        if (curr.mGpButton != eGamepadButton_null)
        {
            mGpButtonActions[curr.mGpButton].mActions[curr.mActionGroup] = curr.mAction;
        }

        mActionToKeys[curr.mAction].mKeycode = curr.mKeycode;
        mActionToKeys[curr.mAction].mGpButton = curr.mGpButton;
    }
}

void InputActionsMapping::LoadConfig(cxx::json_document_node& configNode)
{
    Clear();

    cxx::json_get_attribute(configNode, "gamepad_id", mGamepadID);

    // scan actions
    if (cxx::json_node_array actionsNode = configNode["actions"])
    {
        for (cxx::json_node_object currActionNode = actionsNode.first_child(); 
            currActionNode; 
            currActionNode = currActionNode.next_sibling())
        {
            eInputAction action = eInputAction_null;
            if (!cxx::json_get_attribute(currActionNode, "action", action) || action == eInputAction_null)
                continue; 

            eKeycode keycode = eKeycode_null;
            cxx::json_get_attribute(currActionNode, "keycode", keycode);
            debug_assert(keycode < eKeycode_COUNT);

            eGamepadButton gpButton = eGamepadButton_null;
            cxx::json_get_attribute(currActionNode, "gpbutton", gpButton);
            debug_assert(gpButton < eGamepadButton_COUNT);

            eInputActionsGroup actionGroup = GetInputActionsGroup(action);
            debug_assert(actionGroup < eInputActionsGroup_COUNT);

            // map action
            mKeycodeActions[keycode].mActions[actionGroup] = action;
            mGpButtonActions[gpButton].mActions[actionGroup] = action;

            // map action
            mActionToKeys[action].mKeycode = keycode;
            mActionToKeys[action].mGpButton = gpButton;
        }
    }
}

void InputActionsMapping::SaveConfig(cxx::json_document_node& configNode)
{
    // todo
}

eInputAction InputActionsMapping::GetAction(eInputActionsGroup group, eKeycode keycode) const
{
    debug_assert(group < eInputActionsGroup_COUNT);
    debug_assert(keycode < eKeycode_COUNT);
    return mKeycodeActions[keycode].mActions[group];
}

eInputAction InputActionsMapping::GetAction(eInputActionsGroup group, eGamepadButton gpButton) const
{
    debug_assert(group < eInputActionsGroup_COUNT);
    debug_assert(gpButton < eGamepadButton_COUNT);
    return mGpButtonActions[gpButton].mActions[group];
}

void InputActionsMapping::SetAction(eInputActionsGroup group, eKeycode keycode, eInputAction action)
{
    debug_assert(group < eInputActionsGroup_COUNT);
    debug_assert(keycode < eKeycode_COUNT);
    debug_assert(action < eInputAction_COUNT);
    mKeycodeActions[keycode].mActions[group] = action;
    mActionToKeys[action].mKeycode = keycode;
}

void InputActionsMapping::SetAction(eInputActionsGroup group, eGamepadButton gpButton, eInputAction action)
{
    debug_assert(group < eInputActionsGroup_COUNT);
    debug_assert(gpButton < eGamepadButton_COUNT);
    debug_assert(action < eInputAction_COUNT);
    mGpButtonActions[gpButton].mActions[group] = action;
    mActionToKeys[action].mGpButton = gpButton;
}

eInputActionsGroup InputActionsMapping::GetInputActionsGroup(eInputAction action) const
{
    switch (action)
    {
        case eInputAction_TurnLeft:
        case eInputAction_TurnRight:
        case eInputAction_Jump:
        case eInputAction_WalkForward:
        case eInputAction_WalkBackward:
        case eInputAction_Run:
        case eInputAction_Shoot:
        case eInputAction_EnterCar:
        case eInputAction_EnterCarAsPassenger:
            return eInputActionsGroup_OnFoot;

        case eInputAction_HandBrake:
        case eInputAction_Accelerate:
        case eInputAction_Reverse:
        case eInputAction_SteerLeft:
        case eInputAction_SteerRight:
        case eInputAction_Horn:
        case eInputAction_LeaveCar:
            return eInputActionsGroup_InCar;
    }
    return eInputActionsGroup_Common;
}
