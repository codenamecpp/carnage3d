#pragma once

class DebugRenderer;

// This class generates randomly wander pedestrians and vehicles on currently visible area on map
class TrafficManager final: public cxx::noncopyable
{
    friend class GameCheatsWindow;

public:
    TrafficManager();

    // Start or stop traffic generators
    void StartupTraffic();
    void CleanupTraffic();

    void UpdateFrame();
    void DebugDraw(DebugRenderer& debugRender);

    int CountTrafficPedestrians() const;
    int CountTrafficCars() const;

private:
    // traffic pedestrians generation
    void GeneratePeds();
    void GenerateTrafficPeds(int pedsCount, GameCamera& view);
    void RemoveOffscreenPeds();
    int GetPedsToGenerateCount(GameCamera& view) const;

    // traffic cars generation
    void GenerateCars();
    void GenerateTrafficCars(int carsCount, GameCamera& view);
    void RemoveOffscreenCars();
    int GetCarsToGenerateCount(GameCamera& view) const;

    // traffic objects generation
    Pedestrian* GenerateRandomTrafficCarDriver(Vehicle* vehicle);
    Pedestrian* GenerateRandomTrafficPedestrian(int posx, int posy, int posz);
    Pedestrian* GenerateHareKrishnas(int posx, int posy, int posz);
    Vehicle* GenerateRandomTrafficCar(int posx, int posy, int posz);

    // attempt to remove traffic pedestrian or vehicle
    bool TryRemoveTrafficPed(Pedestrian* ped);
    bool TryRemoveTrafficCar(Vehicle* car);

private:
    float mLastGenPedsTime = 0.0;
    float mLastGenCarsTime = 0.0f;
    float mLastGenHareKrishnasTime = 0.0f;

    // buffers
    struct CandidatePos
    {
        int mMapX;
        int mMapY;
        int mMapLayer;
    };
    std::vector<CandidatePos> mCandidatePosArray;
};

extern TrafficManager gTrafficManager;