#pragma once

#include "CityScapeData.h"

// top level game application controller
class CarnageGame final: public cxx::noncopyable
{
public:
    // Setup resources and switch to initial game state
    bool Initialize();

    // Cleanup current state and finish game
    void Deinit();

    // Common processing
    void UpdateFrame(Timespan deltaTime);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

public:
    CityScapeData mCityScape;
};

extern CarnageGame gCarnageGame;