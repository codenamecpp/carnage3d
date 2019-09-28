#pragma once

#include "GameDefs.h"

// defines pedestrian control interface
class PedestrianControl final
{
public:
    PedestrianControl(Pedestrian& pedestrian);
    void ResetControl();
    void SetTurnLeft(bool turnEnabled);
    void SetTurnRight(bool turnEnabled);
    bool IsTurnAround() const;
    void SetWalkForward(bool walkEnabled);
    void SetWalkBackward(bool walkEnabled);
    void SetRunning(bool runEnabled);
    bool IsMoves() const;

public:
    Pedestrian& mPedestrian;

    bool mTurnLeft;
    bool mTurnRight;
    bool mWalkForward;
    bool mWalkBackward;
    bool mRunning;
};