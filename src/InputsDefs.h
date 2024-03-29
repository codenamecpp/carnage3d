#pragma once

enum 
{
    KEYMOD_SHIFT    = GLFW_MOD_SHIFT,
    KEYMOD_CTRL     = GLFW_MOD_CONTROL,
    KEYMOD_ALT      = GLFW_MOD_ALT,
};

// mouse buttons
enum eMButton
{
    eMButton_null, // invalid mbutton

    eMButton_LEFT,
    eMButton_RIGHT,
    eMButton_MIDDLE,
    eMButton_COUNT
};

decl_enum_strings(eMButton);

// keys
enum eKeycode
{
    eKeycode_null, // invalid keycode

    eKeycode_ESCAPE,
    eKeycode_SPACE,
    eKeycode_PAGE_UP,
    eKeycode_PAGE_DOWN,
    eKeycode_HOME,
    eKeycode_END,
    eKeycode_INSERT,
    eKeycode_DELETE,
    eKeycode_RIGHT_CTRL,
    eKeycode_LEFT_CTRL,
    eKeycode_BACKSPACE,
    eKeycode_ENTER,
    eKeycode_TAB,
    eKeycode_TILDE,
    eKeycode_SHIFT,

    eKeycode_F1,
    eKeycode_F2,
    eKeycode_F3,
    eKeycode_F4,
    eKeycode_F5,
    eKeycode_F6,
    eKeycode_F7,
    eKeycode_F8,
    eKeycode_F9,
    eKeycode_F10,
    eKeycode_F11,
    eKeycode_F12,

    eKeycode_A,
    eKeycode_B,
    eKeycode_C,
    eKeycode_D,
    eKeycode_E,
    eKeycode_F,
    eKeycode_G,
    eKeycode_H,
    eKeycode_I,
    eKeycode_J,
    eKeycode_K,
    eKeycode_L,
    eKeycode_M,
    eKeycode_N,
    eKeycode_O,
    eKeycode_P,
    eKeycode_Q,
    eKeycode_R,
    eKeycode_S,
    eKeycode_T,
    eKeycode_U,
    eKeycode_V,
    eKeycode_W,
    eKeycode_X,
    eKeycode_Y,
    eKeycode_Z,

    eKeycode_0,
    eKeycode_1,
    eKeycode_2,
    eKeycode_3,
    eKeycode_4,
    eKeycode_5,
    eKeycode_6,
    eKeycode_7,
    eKeycode_8,
    eKeycode_9,
        
    eKeycode_LEFT,
    eKeycode_RIGHT,
    eKeycode_UP,
    eKeycode_DOWN,

    eKeycode_COUNT
};

decl_enum_strings(eKeycode);

// gamepad inputs
enum eGamepadButton
{
    eGamepadButton_null, // invalid button

    eGamepadButton_A,
    eGamepadButton_B,
    eGamepadButton_X,
    eGamepadButton_Y,
    eGamepadButton_LeftBumper,
    eGamepadButton_RightBumper,
    eGamepadButton_Back,
    eGamepadButton_Start,
    eGamepadButton_Guide,
    eGamepadButton_LeftThumb,
    eGamepadButton_RightThumb,
    eGamepadButton_DPAD_Up,
    eGamepadButton_DPAD_Right,
    eGamepadButton_DPAD_Down,
    eGamepadButton_DPAD_Left,
    eGamepadButton_LeftTrigger,
    eGamepadButton_RightTrigger,

    eGamepadButton_COUNT
};

decl_enum_strings(eGamepadButton);

// gamepad controller id
enum eGamepadID
{
    eGamepadID_Gamepad1,
    eGamepadID_Gamepad2,
    eGamepadID_Gamepad3,
    eGamepadID_Gamepad4,

    eGamepadID_COUNT,
};

decl_enum_strings(eGamepadID);

// Human player input actions
enum eInputAction
{
    eInputAction_null, // not an action

    // common
    eInputAction_NextWeapon,
    eInputAction_PrevWeapon,

    // on foot
    eInputAction_TurnLeft,
    eInputAction_TurnRight,
    eInputAction_Jump,
    eInputAction_WalkForward,
    eInputAction_WalkBackward,
    eInputAction_Run, // overrides walk_forward and walk_backward
    eInputAction_Shoot,
    eInputAction_EnterCar,
    eInputAction_EnterCarAsPassenger,
    eInputAction_Special,

    // in car
    eInputAction_HandBrake,
    eInputAction_Accelerate,
    eInputAction_Reverse,
    eInputAction_SteerLeft,
    eInputAction_SteerRight,
    eInputAction_Horn,
    eInputAction_LeaveCar,

    eInputAction_COUNT
};

decl_enum_strings(eInputAction);

enum eInputActionsGroup
{
    eInputActionsGroup_Common,
    eInputActionsGroup_OnFoot,
    eInputActionsGroup_InCar,

    eInputActionsGroup_COUNT
};

decl_enum_strings(eInputActionsGroup);

// gamepad state
struct GamepadState
{
public:
    GamepadState() = default;
public:
    bool mButtons[eGamepadButton_COUNT];
    bool mPresent;
};

// input events
struct BaseInputEvent
{
public:
    BaseInputEvent() = default;
    // Set consumed flag
    void SetConsumed(bool isConsumed = true)
    {
        mConsumed = isConsumed;
    }
public:
    bool mConsumed = false; // Whether event is already consumed
};

struct KeyInputEvent: public BaseInputEvent
{
public:
    KeyInputEvent() = default;
    KeyInputEvent(eKeycode argKeycode, int argScancode, int argMods, bool argIsPressed)
        : mKeycode(argKeycode)
        , mScancode(argScancode)
        , mMods(argMods)
        , mPressed(argIsPressed)
    {
    }
    // test whether modifier keys are pressed
    bool HasMods(int bits) const
    {
        return (mMods & bits) == bits;
    }
    // test whether specific key was pressed or released
    inline bool HasPressed(eKeycode keycode) const
    {
        return mPressed && (keycode == mKeycode);
    }
    inline bool HasReleased(eKeycode keycode) const
    {
        return !mPressed && (keycode == mKeycode);
    }
public:
    eKeycode mKeycode;
    int mScancode;
    int mMods;
    bool mPressed; 
};

struct KeyCharEvent: public BaseInputEvent
{
public:
    KeyCharEvent() = default;
    KeyCharEvent(unsigned int unicodeChar): mUnicodeChar(unicodeChar)
    {
    }
public:
    unsigned int mUnicodeChar;
};

struct MouseButtonInputEvent: public BaseInputEvent
{
public:
    MouseButtonInputEvent() = default;
    MouseButtonInputEvent(eMButton argButton, int argMods, bool argIsPressed)
        : mButton(argButton)
        , mMods(argMods)
        , mPressed(argIsPressed)
    {
    }
public:
    eMButton mButton;
    int mMods;
    bool mPressed;
};

struct MouseMovedInputEvent: public BaseInputEvent
{
public:
    MouseMovedInputEvent() = default;
    MouseMovedInputEvent(int argPositionX, int argPositionY)
        : mCursorPositionX(argPositionX)
        , mCursorPositionY(argPositionY)
        , mDeltaX()
        , mDeltaY()
    {
    }
public:
    int mCursorPositionX;
    int mCursorPositionY;
    int mDeltaX;
    int mDeltaY;
};

struct MouseScrollInputEvent: public BaseInputEvent
{
public:
    MouseScrollInputEvent() = default;
    MouseScrollInputEvent(int argScrollX, int argScrollY)
        : mScrollX(argScrollX)
        , mScrollY(argScrollY)
    {
    }
public:
    int mScrollX;
    int mScrollY;
};

struct GamepadInputEvent: public BaseInputEvent
{
public:
    GamepadInputEvent() = default;
    GamepadInputEvent(int gamepad, eGamepadButton button, bool pressed)
        : mButton(button)
        , mGamepad(gamepad)
        , mPressed(pressed)
    {
    }
public:
    int mGamepad;
    eGamepadButton mButton;
    bool mPressed;
};

// Input events handler interfacce
class InputEventsHandler
{
public:
    virtual ~InputEventsHandler()
    {
    }

    // Process input event
    // @param inputEvent: Event data
    virtual void InputEvent(MouseButtonInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(MouseMovedInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(MouseScrollInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(KeyInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(KeyCharEvent& inputEvent)
    {
    }
    virtual void InputEvent(GamepadInputEvent& inputEvent)
    {
    }

    // Process the special case when recent input event was intercepted by someone else
    virtual void InputEventLost()
    {
    }
};


