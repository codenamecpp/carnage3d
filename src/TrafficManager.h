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
    void DebugDraw(DebugRenderer& debugRender);

    int CountTrafficPedestrians() const;

private:
    void GeneratePedestrians();
    void GenerateTrafficPedestrians(int pedsCount, RenderView& view);
    void RemoveOffscreenPedestrians();
    int GetPedestriansToGenerateCount(RenderView& view) const;

private:
    float mLastGenPedestriansTime = 0.0;

    // buffers
    struct CandidatePedestrianPos
    {
        int mMapX;
        int mMapY;
        int mMapLayer;
    };
    std::vector<CandidatePedestrianPos> mCandidatePedsPosArray;
};

extern TrafficManager gTrafficManager;