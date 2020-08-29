#pragma once

class DebugRenderer;
class RenderView;

// This class generates randomly wander pedestrians and vehicles on currently visible area on map
class TrafficManager final: public cxx::noncopyable
{
public:
    TrafficManager();

    // Start or stop traffic generators
    void StartupTraffic();
    void CleanupTraffic();

    void UpdateFrame();
    void DrawDebug(DebugRenderer& debugRender);

private:
    void GeneratePedestrians(bool offscreenOnly);
    void ScanOffscreenPedestrians();

    bool IsTrafficPedestriansLimitReached() const;

private:
    cxx::randomizer mRand;

    // pedestrian generation params
    int mGenMaxPedestrians = 30; // max pedestrians
    int mGenMaxPedestriansPerIteration = 8; // max generate pedestrians on single step
    int mGenPedestriansChance = 60;
    int mGenPedestriansMinDistance = 6; // min distance from player character, blocks
    int mGenPedestriansMaxDistance = 9; // blocks
    float mGenPedestriansCooldownTime = 1.0f; // seconds
    float mLastGenPedestriansTime = 0.0;
};

extern TrafficManager gTrafficManager;