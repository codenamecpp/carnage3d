#pragma once

#include "GameMapManager.h"
#include "GameObjectsManager.h"
#include "HumanPlayer.h"

// top level game application controller
class CarnageGame final: public InputEventsHandler
{
public:
    // gamestate
    HumanPlayer* mHumanPlayers[GAME_MAX_PLAYERS];
    eGtaGameVersion mGameVersion = eGtaGameVersion_Full;
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
    void SetupHumanPlayer(int playerIndex, Pedestrian* pedestrian);
    void DeleteHumanPlayer(int playerIndex);

    void SetupScreenLayout();

    // Get index or human player in players list
    // @returns -1 on error
    int GetHumanPlayerIndex(const HumanPlayer* controller) const;
    int GetHumanPlayersCount() const;

    // Debug stuff
    void DebugChangeMap(const std::string& mapName);

private:
    bool SetInputActionsFromConfig();
    bool DetectGameVersion();

    std::string GetTextsLanguageFileName(const std::string& languageID) const;

    bool StartScenario(const std::string& mapName);
    void ShutdownCurrentScenario();

private:
    std::string mDebugChangeMapName;
};

extern CarnageGame gCarnageGame;