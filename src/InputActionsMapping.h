#pragma once

#include "InputsDefs.h"

// Contains input actions mapping for human player
class InputActionsMapping
{
public:
    InputActionsMapping();

    // Setup actions mapping from config node
    void LoadConfig(cxx::json_document_node& configNode);
    void SaveConfig(cxx::json_document_node& configNode);

    void SetDefaults();
    void Clear();

    // Get action mapped to input keycode or gamepad button
    // @returns eInputAction_null if no action is mapped to input key
    eInputAction GetAction(eInputActionsGroup group, eKeycode keycode) const;
    eInputAction GetAction(eInputActionsGroup group, eGamepadButton gpButton) const;

    // Set action mapped to input keycode or gamepad button
    void SetAction(eInputActionsGroup group, eKeycode keycode, eInputAction action);
    void SetAction(eInputActionsGroup group, eGamepadButton gpButton, eInputAction action);

    // Get input group by action identifier
    eInputActionsGroup GetInputActionsGroup(eInputAction action) const;

public:
    // readonly
    struct ActionsByGroup
    {
        eInputAction mActions[eInputActionsGroup_COUNT];
    };
    ActionsByGroup mKeycodeActions[eKeycode_COUNT]; // key to action mapping
    ActionsByGroup mGpButtonActions[eGamepadButton_COUNT]; // gamepad button to action mapping

    struct ActionToKey
    {
        eKeycode mKeycode;
        eGamepadButton mGpButton;
    };
    ActionToKey mActionToKeys[eInputAction_COUNT];

    eGamepadID mGamepadID = eGamepadID_Gamepad1;
};