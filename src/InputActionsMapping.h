#pragma once

#include "GameDefs.h"

class InputActionsMapping
{
public:

public:
    eInputControllerType mControllerType = eInputControllerType_None;
    // keyboard mapping
    ePedestrianAction mKeycodesToActions[eKeycode_COUNT];
    eKeycode mActionsToKeycodes[ePedestrianAction_COUNT];
    // gamepad mapping
    ePedestrianAction mGamepadButtonsToActions[eGamepadButton_COUNT];
    eGamepadButton mActionsToGamepadButtons[ePedestrianAction_COUNT];
};