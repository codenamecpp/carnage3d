#pragma once

// Key codes
enum
{
    KEYCODE_ESCAPE      = GLFW_KEY_ESCAPE,
    KEYCODE_SPACE       = GLFW_KEY_SPACE,
    KEYCODE_PAGE_UP     = GLFW_KEY_PAGE_UP,
    KEYCODE_PAGE_DOWN   = GLFW_KEY_PAGE_DOWN,
    KEYCODE_HOME        = GLFW_KEY_HOME,
    KEYCODE_END         = GLFW_KEY_END,
    KEYCODE_INSERT      = GLFW_KEY_INSERT,
    KEYCODE_DELETE      = GLFW_KEY_DELETE,
    KEYCODE_RIGHT_CTRL  = GLFW_KEY_RIGHT_CONTROL,
    KEYCODE_LEFT_CTRL   = GLFW_KEY_LEFT_CONTROL,
    KEYCODE_BACKSPACE   = GLFW_KEY_BACKSPACE,
    KEYCODE_ENTER       = GLFW_KEY_ENTER,
    KEYCODE_TAB         = GLFW_KEY_TAB,
    KEYCODE_TILDE       = GLFW_KEY_GRAVE_ACCENT,

    KEYCODE_F1          = GLFW_KEY_F1,
    KEYCODE_F2          = GLFW_KEY_F2,
    KEYCODE_F3          = GLFW_KEY_F3,
    KEYCODE_F4          = GLFW_KEY_F4,
    KEYCODE_F5          = GLFW_KEY_F5,
    KEYCODE_F6          = GLFW_KEY_F6,
    KEYCODE_F7          = GLFW_KEY_F7,
    KEYCODE_F8          = GLFW_KEY_F8,
    KEYCODE_F9          = GLFW_KEY_F9,
    KEYCODE_F10         = GLFW_KEY_F10,
    KEYCODE_F11         = GLFW_KEY_F11,
    KEYCODE_F12         = GLFW_KEY_F12,

    // Letters
    KEYCODE_A           = GLFW_KEY_A,
    KEYCODE_C           = GLFW_KEY_C,
    KEYCODE_V           = GLFW_KEY_V,
    KEYCODE_X           = GLFW_KEY_X,
    KEYCODE_W           = GLFW_KEY_W,
    KEYCODE_D           = GLFW_KEY_D,
    KEYCODE_S           = GLFW_KEY_S,
    KEYCODE_Y           = GLFW_KEY_Y,
    KEYCODE_Z           = GLFW_KEY_Z,
    KEYCODE_R           = GLFW_KEY_R,

    // digits
    KEYCODE_0           = GLFW_KEY_0,
    KEYCODE_1           = GLFW_KEY_1,
    KEYCODE_2           = GLFW_KEY_2,
    KEYCODE_3           = GLFW_KEY_3,
    KEYCODE_4           = GLFW_KEY_4,
    KEYCODE_5           = GLFW_KEY_5,
    KEYCODE_6           = GLFW_KEY_6,
    KEYCODE_7           = GLFW_KEY_7,
    KEYCODE_8           = GLFW_KEY_8,
    KEYCODE_9           = GLFW_KEY_9,
        
    // Arrow keys
    KEYCODE_LEFT        = GLFW_KEY_LEFT,
    KEYCODE_RIGHT       = GLFW_KEY_RIGHT,
    KEYCODE_UP          = GLFW_KEY_UP,
    KEYCODE_DOWN        = GLFW_KEY_DOWN,

    // mouse buttons
    MBUTTON_LEFT        = GLFW_MOUSE_BUTTON_LEFT,
    MBUTTON_RIGHT       = GLFW_MOUSE_BUTTON_RIGHT,
    MBUTTON_MIDDLE      = GLFW_MOUSE_BUTTON_MIDDLE,

    // Key mods
    KEYMOD_SHIFT        = GLFW_MOD_SHIFT,
    KEYMOD_CTRL         = GLFW_MOD_CONTROL,
    KEYMOD_ALT          = GLFW_MOD_ALT
};

// Input events

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

struct KeyInputEvent : public BaseInputEvent
{
public:
    KeyInputEvent() = default;
    KeyInputEvent(int argKeycode, int argScancode, int argMods, bool argIsPressed)
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
public:
    int mKeycode;
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

struct MouseButtonInputEvent : public BaseInputEvent
{
public:
    MouseButtonInputEvent() = default;
    MouseButtonInputEvent(int argButton, int argMods, bool argIsPressed)
        : mButton(argButton)
        , mMods(argMods)
        , mPressed(argIsPressed)
    {
    }
public:
    int mButton;
    int mMods;
    bool mPressed;
};

struct MouseMovedInputEvent : public BaseInputEvent
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

struct MouseScrollInputEvent : public BaseInputEvent
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

//////////////////////////////////////////////////////////////////////////
// Input State Holder
//////////////////////////////////////////////////////////////////////////

class Inputs final: public cxx::noncopyable
{
public:
    bool mMouseButtons[GLFW_MOUSE_BUTTON_LAST];
    bool mKeyboardKeys[GLFW_KEY_LAST];
    int mCursorPositionX;
    int mCursorPositionY;

public:
    Inputs();

    // Process input event
    // @param inputEvent: Event data
    void HandleEvent(MouseButtonInputEvent& inputEvent);
    void HandleEvent(MouseMovedInputEvent& inputEvent);
    void HandleEvent(MouseScrollInputEvent& inputEvent);
    void HandleEvent(KeyInputEvent& inputEvent);
    void HandleEvent(KeyCharEvent& inputEvent);

    // Reset all keys and mouse buttons state
    void Cleanup();

    // Set current mouse position
    // @param positionx: position x
    // @param positiony: position y
    void SetMousePosition(int positionx, int positiony)
    {
        mCursorPositionX = positionx;
        mCursorPositionY = positiony;
    }
    // Set current mouse button state
    // @param button: mouse button index
    // @param buttonState: button pressed/released
    void SetMouseButtonState(unsigned button, bool state)
    {
        debug_assert (button < GLFW_MOUSE_BUTTON_LAST);
        mMouseButtons[button] = state;
    }
    // Set current key state
    // @param keycode: keycode
    // @param keystate: key pressed/released
    void SetKeyState(unsigned keycode, bool state)
    {
        debug_assert(keycode < GLFW_KEY_LAST);
        mKeyboardKeys[keycode] = state;
    }
    // Get current key state
    // @param keycode: keycode
    bool GetKeyState(unsigned keycode) const 
    {
        debug_assert(keycode < GLFW_KEY_LAST);
        return mKeyboardKeys[keycode];
    }
    bool GetMouseButtonL() const { return mMouseButtons[GLFW_MOUSE_BUTTON_LEFT]; }
    bool GetMouseButtonR() const { return mMouseButtons[GLFW_MOUSE_BUTTON_RIGHT]; }
    bool GetMouseButtonM() const { return mMouseButtons[GLFW_MOUSE_BUTTON_MIDDLE]; }
    // Get current mouse button state
    // @param button: mouse button index
    bool GetMouseButton(unsigned button) const 
    {
        debug_assert(button < GLFW_MOUSE_BUTTON_LAST);
        return mMouseButtons[button];
    }
};

extern Inputs gInputs;