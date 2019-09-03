#pragma once

// defines manager that hold all game logic parameters
class GameRules
{
public:
    GameRules();

    // reset all rules to default values
    void LoadDefaults();

public:
    // pedestrians
    float mPedestrianTurnSpeed; // degrees per second
    float mPedestrianWalkSpeed; // in blocks per second
    float mPedestrianRunSpeed; // in blocks per second
    float mPedestrianBackWalkSpeed; // in blocks per second 
};

extern GameRules gGameRules;