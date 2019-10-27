#pragma once

enum //key mods
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
    eKeycode_C,
    eKeycode_F,
    eKeycode_V,
    eKeycode_X,
    eKeycode_W,
    eKeycode_D,
    eKeycode_S,
    eKeycode_Y,
    eKeycode_Z,
    eKeycode_R,

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

//////////////////////////////////////////////////////////////////////////
// Input State Holder
//////////////////////////////////////////////////////////////////////////

class Inputs final: public cxx::noncopyable
{
public:
    bool mMouseButtons[eMButton_COUNT];
    bool mKeyboardKeys[eKeycode_COUNT];
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
    void SetMouseButtonState(eMButton button, bool state)
    {
        debug_assert(button < eMButton_COUNT && button > eMButton_null);
        mMouseButtons[button] = state;
    }
    // Set current key state
    // @param keycode: keycode
    // @param keystate: key pressed/released
    void SetKeyState(eKeycode keycode, bool state)
    {
        debug_assert(keycode < eKeycode_COUNT && keycode > eKeycode_null);
        mKeyboardKeys[keycode] = state;
    }
    // Get current key state
    // @param keycode: keycode
    bool GetKeyState(eKeycode keycode) const 
    {
        debug_assert(keycode < eKeycode_COUNT && keycode > eKeycode_null);
        return mKeyboardKeys[keycode];
    }
    bool GetMouseButtonL() const { return mMouseButtons[eMButton_LEFT]; }
    bool GetMouseButtonR() const { return mMouseButtons[eMButton_RIGHT]; }
    bool GetMouseButtonM() const { return mMouseButtons[eMButton_MIDDLE]; }
    // Get current mouse button state
    // @param button: mouse button index
    bool GetMouseButton(eMButton button) const 
    {
        debug_assert(button < eMButton_COUNT && button > eMButton_null);
        return mMouseButtons[button];
    }
};

extern Inputs gInputs;