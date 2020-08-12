#pragma once

#include "GameMapManager.h"
#include "GameObjectsManager.h"
#include "HumanCharacterController.h"
#include "HumanCharacterView.h"

// top level game application controller
class CarnageGame final: public cxx::noncopyable
{
public:
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
    HumanCharacterSlot mHumanSlot[GAME_MAX_PLAYERS];
    int mNumPlayers = 0;

    cxx::randomizer mGameRand;

public:
    // Setup resources and switch to initial game state
    bool Initialize();

    // Cleanup current state and finish game
    void Deinit();

    // Common processing
    void UpdateFrame();
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);

    // Initialize player data
    void SetupHumanCharacter(int playerIndex, Pedestrian* pedestrian);
    void SetupScreenLayout(int playersCount);

    // Get player index from human char controller
    // @returns -1 on error
    int GetPlayerIndex(const HumanCharacterController* controller) const;

    // Debug stuff
    void DebugChangeMap(const std::string& mapName);

private:
    bool SetInputActionsFromConfig();

    bool StartScenario(const std::string& mapName);
    void ShutdownCurrentScenario();
};

extern CarnageGame gCarnageGame;