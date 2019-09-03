#pragma once

#include "GameDefs.h"

// defines generic city pedestrian
class Pedestrian final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    glm::vec3 mPosition; // real position in space
    glm::vec3 mPrevPosition;
    float mRotation;
    float mPrevRotation;
    float mSphereRadius; // bounding sphere info

    SpriteAnimation mAnimation;

    bool mDead;

public:
    Pedestrian();

    // process current animation and logic
    void UpdateFrame(Timespan deltaTime);
};

//////////////////////////////////////////////////////////////////////////

// defines peds manager class
class PedestrianManager final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    std::vector<Pedestrian*> mActivePedsList;
    std::vector<Pedestrian*> mDestroyPedsList;

public:
    bool Initialize();
    void Deinit();

    void UpdateFrame(Timespan deltaTime);

    // add random pedestrian to map at specific location
    // @param position: Real world position
    Pedestrian* CreateRandomPed(const glm::vec3& position);

    // will remove ped from active list and put it to destroy list, does not destroy immediately
    // @param ped: Pedestrian instance
    void RemovePedestrian(Pedestrian* ped);

private:
    void DestroyPendingPeds();
    void RemoveOffscreenPeds();
    void AddToActiveList(Pedestrian* ped);

private:
    cxx::object_pool<Pedestrian> mPedsPool;
};