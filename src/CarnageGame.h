#pragma once

#include "GameMapManager.h"
#include "GameObjectsManager.h"
#include "HumanCharacterController.h"
#include "HumanCharacterView.h"

// top level game application controller
class CarnageGame final: public cxx::noncopyable
{
public:
    GameObjectsManager mObjectsManager;

    // temporary
    struct HumanCharacterSlot
    {
    public:
        HumanCharacterSlot() = default;        
    public:
        HumanCharacterController mCharController;
        HumanCharacterView mCharView;
        Pedestrian* mCharPedestrian = nullptr;
    };

    // gamestate
    HumanCharacterSlot mHumanCharacters[GAME_MAX_PLAYERS];

    Timespan mGameTime;

    cxx::randomizer mGameRand;

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
    void InputEvent(GamepadInputEvent& inputEvent);

    // initialize player data
    void SetupHumanCharacter(int playerIndex, Pedestrian* pedestrian);
    void SetupScreenLayout(int playersCount);

private:
    bool SetInputActionsFromConfig();
};

extern CarnageGame gCarnageGame;