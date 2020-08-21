#pragma once

#include "GameMapManager.h"
#include "GameObjectsManager.h"
#include "HumanCharacterController.h"
#include "HumanCharacterView.h"

// top level game application controller
class CarnageGame final: public InputEventsHandler
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

    // override InputEventsHandler
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(MouseMovedInputEvent& inputEvent) override;
    void InputEvent(MouseScrollInputEvent& inputEvent) override;
    void InputEvent(KeyCharEvent& inputEvent) override;
    void InputEvent(GamepadInputEvent& inputEvent) override;
    void InputEventLost() override;

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