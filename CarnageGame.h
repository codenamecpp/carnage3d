#pragma once

#include "CityScapeData.h"
#include "TopDownCameraController.h"
#include "Pedestrian.h"

// top level game application controller
class CarnageGame final: public cxx::noncopyable
{
public:
    CityScapeData mCityScape;
    PedestrianManager mPedsManager;
    TopDownCameraController mTopDownCameraController;

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
};

extern CarnageGame gCarnageGame;