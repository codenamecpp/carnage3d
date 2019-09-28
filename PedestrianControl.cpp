#include "stdafx.h"
#include "PedestrianControl.h"

PedestrianControl::PedestrianControl(Pedestrian& pedestrian)
    : mPedestrian(pedestrian)
{
    ResetControl();
}

void PedestrianControl::ResetControl()
{
    mTurnLeft = false;
    mTurnRight = false;
    mWalkBackward = false;
    mWalkForward = false;
    mRunning = false;
}

void PedestrianControl::SetTurnLeft(bool turnEnabled)
{
    mTurnLeft = turnEnabled;
}

void PedestrianControl::SetTurnRight(bool turnEnabled)
{
    mTurnRight = turnEnabled;
}

void PedestrianControl::SetWalkForward(bool walkEnabled)
{
    mWalkForward = walkEnabled;
}

void PedestrianControl::SetWalkBackward(bool walkEnabled)
{
    mWalkBackward = walkEnabled;
}

bool PedestrianControl::IsTurnAround() const
{
    return mTurnLeft || mTurnRight;
}

bool PedestrianControl::IsMoves() const
{
    return mWalkBackward || mWalkForward;
}

void PedestrianControl::SetRunning(bool runEnabled)
{
    mRunning = runEnabled;
}
