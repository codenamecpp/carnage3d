#include "stdafx.h"
#include "TrafficManager.h"
#include "DebugRenderer.h"
#include "CarnageGame.h"
#include "TimeManager.h"
#include "AiManager.h"
#include "GameCheatsWindow.h"
#include "AiCharacterController.h"
#include "GameMapHelpers.h"

TrafficManager gTrafficManager;

// get rect of map blocks on screen, in map units
static Rect GetOnScreenMapRect(const GameCamera& view)
{
    glm::ivec2 minBlock = glm::ivec2(Convert::MetersToMapUnits(view.mOnScreenMapArea.mMin));
    glm::ivec2 maxBlock = glm::ivec2(Convert::MetersToMapUnits(view.mOnScreenMapArea.mMax)) + 1;
    return Rect(minBlock.x, minBlock.y, maxBlock.x - minBlock.x, maxBlock.y - minBlock.y);
}

// get top non-air block at map position
// @returns nullptr if all layers are air
static const MapBlockInfo* GetTopMapBlock(int mapx, int mapy, int& outLayer)
{
    for (outLayer = (MAP_LAYERS_COUNT - 1); outLayer >= 0; --outLayer)
    {
        const MapBlockInfo* mapBlock = gGameMap.GetBlockInfo(mapx, mapy, outLayer);
        if (mapBlock->mGroundType != eGroundType_Air)
            return mapBlock;
    }
    return nullptr;
}

static int GetDirectionsCount(const MapBlockInfo& mapBlock)
{
    return (int) mapBlock.mUpDirection + (int) mapBlock.mDownDirection +
        (int) mapBlock.mLeftDirection + (int) mapBlock.mRightDirection;
}

TrafficManager::TrafficManager()
{
}

void TrafficManager::StartupTraffic()
{   
    mGenPedsSinceHareKrishnas = 0;

    mLastGenPedsTime = 0.0f;
    GeneratePeds();

    mLastGenCarsTime = 0.0f;
    GenerateCars();

}

void TrafficManager::CleanupTraffic()
{
    for (Vehicle* currCar: gGameObjectsManager.mVehiclesList)
    {
        TryRemoveTrafficCar(currCar);
    }

    for (Pedestrian* currPedestrian: gGameObjectsManager.mPedestriansList)
    {
        TryRemoveTrafficPed(currPedestrian);
    }

    mHareKrishnasGroup.clear();
}

void TrafficManager::UpdateFrame()
{
    UpdateHareKrishnasGroup();
    GeneratePeds();
    GenerateCars();
}

// get ai controller of character, it is null for human character
static AiCharacterController* GetAiController(Pedestrian* pedestrian)
{
    if (pedestrian && pedestrian->mController && pedestrian->mController->IsControllerTypeAi())
        return static_cast<AiCharacterController*>(pedestrian->mController);

    return nullptr;
}

void TrafficManager::UpdateHareKrishnasGroup()
{
    if (mHareKrishnasGroup.empty())
        return;

    Pedestrian* leader = mHareKrishnasGroup[0];
    if (leader == nullptr || leader->IsMarkedForDeletion() || leader->IsDead())
    {
        if (leader)
        {
            leader->StopGameObjectSound(ePedSfxChannelIndex_Chant);
        }

        // when leader dies followers leave the line and run away
        for (size_t imember = 1; leader && leader->IsDead() && (imember < mHareKrishnasGroup.size()); ++imember)
        {
            Pedestrian* follower = mHareKrishnasGroup[imember];
            AiCharacterController* controller = GetAiController(follower);
            if (controller && !follower->IsDead())
            {
                controller->StartPanic();
            }
        }
        mHareKrishnasGroup.clear();
        return;
    }

    // remove dead followers
    size_t firstChangedMember = mHareKrishnasGroup.size();
    for (size_t imember = mHareKrishnasGroup.size() - 1; imember > 0; --imember)
    {
        Pedestrian* follower = mHareKrishnasGroup[imember];
        if (follower && !follower->IsMarkedForDeletion() && !follower->IsDead())
            continue;

        // chant stops for good when any group member dies
        if (follower && follower->IsDead())
        {
            mIsHareKrishnasChanting = false;
        }
        mHareKrishnasGroup.erase(mHareKrishnasGroup.begin() + imember);
        firstChangedMember = imember;
    }

    // followers behind dead follower move up in the line
    for (size_t imember = firstChangedMember; imember < mHareKrishnasGroup.size(); ++imember)
    {
        if (AiCharacterController* controller = GetAiController(mHareKrishnasGroup[imember]))
        {
            controller->FollowPedestrian(mHareKrishnasGroup[imember - 1]);
        }
    }

    if (!mIsHareKrishnasChanting)
    {
        leader->StopGameObjectSound(ePedSfxChannelIndex_Chant);
    }
    else if (!leader->CheckGameObjectSoundPlaying(ePedSfxChannelIndex_Chant))
    {
        leader->StartGameObjectSound(ePedSfxChannelIndex_Chant, eSfxSampleType_Level, SfxLevel_HareKrishnasChant, SfxFlags_None);
    }
}

void TrafficManager::RemoveHareKrishnasGroup()
{
    for (size_t imember = 0; imember < mHareKrishnasGroup.size(); ++imember)
    {
        Pedestrian* member = mHareKrishnasGroup[imember];
        if (member == nullptr)
            continue;

        member->StopGameObjectSound(ePedSfxChannelIndex_Chant);
        TryRemoveTrafficPed(member);
    }
    mHareKrishnasGroup.clear();
}

void TrafficManager::DebugDraw(DebugRenderer& debugRender, GameCamera& view)
{
    if (gGameCheatsWindow.mEnableDrawTrafficLanes)
    {
        DebugDrawTrafficLanes(debugRender, view);
    }
}

void TrafficManager::DebugDrawTrafficLanes(DebugRenderer& debugRender, GameCamera& view)
{
    const float DrawLiftHeight = 0.05f; // meters, keep lines above road surface
    const float ArrowHeadLength = 0.2f; // map units
    const float ArrowHeadWidth = 0.12f; // map units

    // same order as block direction flags: up, down, left, right
    static const glm::vec2 Directions[] = { {0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f}, {1.0f, 0.0f} };

    Rect onScreenRect = GetOnScreenMapRect(view);
    int minx = glm::max(onScreenRect.x, 0);
    int miny = glm::max(onScreenRect.y, 0);
    int maxx = glm::min(onScreenRect.x + onScreenRect.w, MAP_DIMENSIONS);
    int maxy = glm::min(onScreenRect.y + onScreenRect.h, MAP_DIMENSIONS);

    for (int iy = miny; iy < maxy; ++iy)
    for (int ix = minx; ix < maxx; ++ix)
    {
        int layer = 0;
        const MapBlockInfo* mapBlock = GetTopMapBlock(ix, iy, layer);
        if (mapBlock == nullptr || mapBlock->mGroundType != eGroundType_Road)
            continue;

        // get world point on road surface, local point is in range [0, 1] within block
        auto GetSurfacePoint = [&](const glm::vec2& local)
        {
            float height = (float) layer;
            if (mapBlock->mSlopeType)
            {
                height += GameMapHelpers::GetSlopeHeight(mapBlock->mSlopeType, local.x, local.y);
            }
            glm::vec3 point = Convert::MapUnitsToMeters(glm::vec3(ix + local.x, height, iy + local.y));
            point.y += DrawLiftHeight;
            return point;
        };

        const glm::vec2 center (0.5f);
        const glm::vec3 centerPoint = GetSurfacePoint(center);

        // mark traffic lights
        if (mapBlock->mTrafficHint == eTrafficHint_TrafficLights)
        {
            debugRender.DrawCube(centerPoint, glm::vec3(0.15f), Color32_Red, false);
        }

        const bool hasDirection[] =
        {
            mapBlock->mUpDirection, mapBlock->mDownDirection, mapBlock->mLeftDirection, mapBlock->mRightDirection
        };
        int directionsCount = GetDirectionsCount(*mapBlock);

        // railway - gray, junction - yellow, single lane - green
        Color32 lineColor = mapBlock->mIsRailway ? Color32_Gray : (directionsCount > 1) ? Color32_Yellow : Color32_Green;

        for (int idir = 0; idir < 4; ++idir)
        {
            if (!hasDirection[idir])
                continue;

            // draw line through block from edge to edge, with arrow head at block center
            const glm::vec2& dir = Directions[idir];
            const glm::vec2 side (-dir.y, dir.x);
            const glm::vec2 headBase = center - dir * ArrowHeadLength;

            debugRender.DrawLine(GetSurfacePoint(center - dir * 0.5f), GetSurfacePoint(center + dir * 0.5f), lineColor, false);
            debugRender.DrawLine(centerPoint, GetSurfacePoint(headBase + side * ArrowHeadWidth), lineColor, false);
            debugRender.DrawLine(centerPoint, GetSurfacePoint(headBase - side * ArrowHeadWidth), lineColor, false);
        }
    }
}

void TrafficManager::GeneratePeds()
{
    if ((mLastGenPedsTime > 0.0f) && 
        (mLastGenPedsTime + gGameParams.mTrafficGenPedsCooldownTime) > gTimeManager.mGameTime)
    {
        return;
    }

    mLastGenPedsTime = gTimeManager.mGameTime;

    RemoveOffscreenPeds();

    if (!gGameCheatsWindow.mEnableTrafficPedsGeneration)
        return;

    for (HumanPlayer* humanPlayer: gCarnageGame.mHumanPlayers)
    {   
        if (humanPlayer == nullptr)
            continue;

        int generatePedsCount = GetPedsToGenerateCount(humanPlayer->mViewCamera);
        if (generatePedsCount > 0)
        {
            GenerateTrafficPeds(generatePedsCount, humanPlayer->mViewCamera);
        }
    }
}

void TrafficManager::RemoveOffscreenPeds()
{
    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenPedsMaxDistance + 1.0f);

    for (Pedestrian* pedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (pedestrian->IsMarkedForDeletion() || !pedestrian->IsTrafficFlag())
            continue;

        // skip vehicle passengers
        if (pedestrian->IsCarPassenger())
            continue;

        // original game never removes hare krishnas group by distance
        if (cxx::contains(mHareKrishnasGroup, pedestrian))
            continue;

        bool isOnScreen = false;
        for (HumanPlayer* humanPlayer: gCarnageGame.mHumanPlayers)
        {   
            if (humanPlayer == nullptr)
                continue;

            cxx::aabbox2d_t onScreenArea = humanPlayer->mViewCamera.mOnScreenMapArea;
            onScreenArea.mMax.x += offscreenDistance;
            onScreenArea.mMax.y += offscreenDistance;
            onScreenArea.mMin.x -= offscreenDistance;
            onScreenArea.mMin.y -= offscreenDistance;

            if (pedestrian->IsOnScreen(onScreenArea))
            {
                isOnScreen = true;
                break;
            }
        }

        if (isOnScreen)
            continue;

        // remove ped
        pedestrian->MarkForDeletion();
    }
}

void TrafficManager::GenerateTrafficPeds(int pedsCount, GameCamera& view)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    ScanCandidatePositions(view, gGameParams.mTrafficGenPedsMaxDistance, [](const MapBlockInfo& mapBlock)
    {
        return (mapBlock.mGroundType == eGroundType_Pawement) && !mapBlock.mIsRailway;
    });

    if (mCandidatePosArray.empty())
        return;

    // shuffle candidates
    random.shuffle(mCandidatePosArray);

    for (int numPedsGenerated = 0; (numPedsGenerated < pedsCount) && !mCandidatePosArray.empty(); ++numPedsGenerated)
    {
        if (!random.random_chance(gGameParams.mTrafficGenPedsChance))
            continue;

        CandidatePos candidate = mCandidatePosArray.back();
        mCandidatePosArray.pop_back();

        bool generateHareKrishnas = false;
        if (mHareKrishnasGroup.empty())
        {
            ++mGenPedsSinceHareKrishnas;
            generateHareKrishnas = (mGenPedsSinceHareKrishnas >= gGameParams.mTrafficGenHareKrishnasPedsInterval);
        }

        if (generateHareKrishnas)
        {
            GenerateHareKrishnas(candidate.mMapX, candidate.mMapLayer, candidate.mMapY);
        }
        else
        {
            // generate normal pedestrian
            GenerateRandomTrafficPedestrian(candidate.mMapX, candidate.mMapLayer, candidate.mMapY);
        }
    }
}

void TrafficManager::ScanCandidatePositions(GameCamera& view, int expandSize, bool (*isCandidate)(const MapBlockInfo& mapBlock))
{
    mCandidatePosArray.clear();

    Rect innerRect = GetOnScreenMapRect(view);
    Rect outerRect = innerRect;
    outerRect.x -= expandSize;
    outerRect.y -= expandSize;
    outerRect.w += expandSize * 2;
    outerRect.h += expandSize * 2;

    // scan blocks around visible area
    for (int iy = outerRect.y; iy < (outerRect.y + outerRect.h); ++iy)
    for (int ix = outerRect.x; ix < (outerRect.x + outerRect.w); ++ix)
    {
        if (innerRect.PointWithin(Point(ix, iy)))
            continue;

        int layer = 0;
        const MapBlockInfo* mapBlock = GetTopMapBlock(ix, iy, layer);
        if (mapBlock == nullptr || !isCandidate(*mapBlock))
            continue;

        CandidatePos candidatePos;
        candidatePos.mMapX = ix;
        candidatePos.mMapY = iy;
        candidatePos.mMapLayer = layer;
        mCandidatePosArray.push_back(candidatePos);
    }
}
int TrafficManager::GetPedsToGenerateCount(GameCamera& view) const
{
    int pedestriansCounter = 0;

    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenPedsMaxDistance * 1.0f);

    cxx::aabbox2d_t onScreenArea = view.mOnScreenMapArea;
    onScreenArea.mMax.x += offscreenDistance;
    onScreenArea.mMax.y += offscreenDistance;
    onScreenArea.mMin.x -= offscreenDistance;
    onScreenArea.mMin.y -= offscreenDistance;

    for (Pedestrian* pedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (!pedestrian->IsTrafficFlag() || pedestrian->IsMarkedForDeletion() || pedestrian->IsCarPassenger())
            continue;

        if (pedestrian->IsOnScreen(onScreenArea))
        {
            ++pedestriansCounter;
        }
    }

    return std::max(0, (gGameParams.mTrafficGenMaxPeds - pedestriansCounter));
}

int TrafficManager::CountTrafficPedestrians() const
{
    int counter = 0;
    for (Pedestrian* pedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (!pedestrian->IsTrafficFlag() || pedestrian->IsMarkedForDeletion() || pedestrian->IsCarPassenger())
            continue;

        ++counter;
    }
    return counter;
}

int TrafficManager::CountTrafficCars() const
{
    int counter = 0;
    for (Vehicle* currVehicle: gGameObjectsManager.mVehiclesList)
    {
        if (currVehicle->IsMarkedForDeletion() || !currVehicle->IsTrafficFlag())
            continue;

        ++counter;
    }
    return counter;
}

int TrafficManager::GetCarsToGenerateCount(GameCamera& view) const
{
    int carsCounter = 0;

    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenCarsMaxDistance * 1.0f);

    cxx::aabbox2d_t onScreenArea = view.mOnScreenMapArea;
    onScreenArea.mMax.x += offscreenDistance;
    onScreenArea.mMax.y += offscreenDistance;
    onScreenArea.mMin.x -= offscreenDistance;
    onScreenArea.mMin.y -= offscreenDistance;

    for (Vehicle* car: gGameObjectsManager.mVehiclesList)
    {
        if (!car->IsTrafficFlag() || car->IsMarkedForDeletion())
            continue;

        if (car->IsOnScreen(onScreenArea))
        {
            ++carsCounter;
        }
    }

    return std::max(0, (gGameParams.mTrafficGenMaxCars - carsCounter));
}

void TrafficManager::GenerateCars()
{
    if ((mLastGenCarsTime > 0.0f) && 
        (mLastGenCarsTime + gGameParams.mTrafficGenCarsCooldownTime) > gTimeManager.mGameTime)
    {
        return;
    }

    mLastGenCarsTime = gTimeManager.mGameTime;

    RemoveOffscreenCars();

    if (!gGameCheatsWindow.mEnableTrafficCarsGeneration)
        return;

    for (HumanPlayer* humanPlayer: gCarnageGame.mHumanPlayers)
    {   
        if (humanPlayer == nullptr)
            continue;

        int generateCarsCount = GetCarsToGenerateCount(humanPlayer->mViewCamera);
        if (generateCarsCount > 0)
        {
            GenerateTrafficCars(generateCarsCount, humanPlayer->mViewCamera);
        }
    }
}

void TrafficManager::GenerateTrafficCars(int carsCount, GameCamera& view)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    // only road blocks with one direction, to know car heading
    ScanCandidatePositions(view, gGameParams.mTrafficGenCarsMaxDistance, [](const MapBlockInfo& mapBlock)
    {
        return (mapBlock.mGroundType == eGroundType_Road) && !mapBlock.mIsRailway && (GetDirectionsCount(mapBlock) == 1);
    });

    if (mCandidatePosArray.empty())
        return;

    // shuffle candidates
    random.shuffle(mCandidatePosArray);

    for (int numCarsGenerated = 0; (numCarsGenerated < carsCount) && !mCandidatePosArray.empty(); ++numCarsGenerated)
    {
        if (!random.random_chance(gGameParams.mTrafficGenCarsChance))
            continue;

        CandidatePos candidate = mCandidatePosArray.back();
        mCandidatePosArray.pop_back();

        GenerateRandomTrafficCar(candidate.mMapX, candidate.mMapLayer, candidate.mMapY);
    }
}
void TrafficManager::RemoveOffscreenCars()
{
    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenCarsMaxDistance + 1.0f);

    for (Vehicle* currentCar: gGameObjectsManager.mVehiclesList)
    {
        if (currentCar->IsMarkedForDeletion() || !currentCar->IsTrafficFlag())
            continue;

        bool isOnScreen = false;
        for (HumanPlayer* humanPlayer: gCarnageGame.mHumanPlayers)
        {   
            if (humanPlayer == nullptr)
                continue;

            cxx::aabbox2d_t onScreenArea = humanPlayer->mViewCamera.mOnScreenMapArea;
            onScreenArea.mMax.x += offscreenDistance;
            onScreenArea.mMax.y += offscreenDistance;
            onScreenArea.mMin.x -= offscreenDistance;
            onScreenArea.mMin.y -= offscreenDistance;

            if (currentCar->IsOnScreen(onScreenArea))
            {
                isOnScreen = true;
                break;
            }
        }

        if (isOnScreen)
            continue;

        TryRemoveTrafficCar(currentCar);
    }
}

Vehicle* TrafficManager::GenerateRandomTrafficCar(int posx, int posy, int posz)
{
    const MapBlockInfo* mapBlock = gGameMap.GetBlockInfo(posx, posz, posy);
  
    glm::vec3 positions(
        Convert::MapUnitsToMeters(posx + 0.5f),
        Convert::MapUnitsToMeters(posy * 1.0f),
        Convert::MapUnitsToMeters(posz + 0.5f)
    );

    float turnAngle = 0.0f;
    if (mapBlock->mUpDirection)
    {
        turnAngle = -90.0f;
    }
    else if (mapBlock->mDownDirection)
    {
        turnAngle = 90.0f;
    }
    else if (mapBlock->mLeftDirection)
    {
        turnAngle = 180.0f;
    }

    // generate car
    cxx::angle_t carHeading(turnAngle, cxx::angle_t::units::degrees);
    positions.y = gGameMap.GetHeightAtPosition(positions);

    // choose car model
    std::vector<VehicleInfo*> models;
    for(VehicleInfo& currModel: gGameMap.mStyleData.mVehicles)
    {
        // filter classes
        if ((currModel.mClassID == eVehicleClass_Tank) || (currModel.mClassID == eVehicleClass_Boat) ||
            (currModel.mClassID == eVehicleClass_Tram) || (currModel.mClassID == eVehicleClass_Train) ||
            (currModel.mClassID == eVehicleClass_BackOfJuggernaut))
        {
            continue;
        }
        // filter models
        if ((currModel.mModelID == eVehicle_Helicopter) || (currModel.mModelID == eVehicle_Ambulance) ||
            (currModel.mModelID == eVehicle_FireTruck) || (currModel.mModelID == eVehicle_ModelCar))
        {
            continue;
        }
        models.push_back(&currModel);
    }
    
    if (models.empty())
        return nullptr;

    // shuffle candidates
    gCarnageGame.mGameRand.shuffle(models);

    Vehicle* vehicle = gGameObjectsManager.CreateVehicle(positions, carHeading, models.front());
    debug_assert(vehicle);
    if (vehicle)
    {
        vehicle->mObjectFlags = (vehicle->mObjectFlags | GameObjectFlags_Traffic);
        // todo: remap

        Pedestrian* carDriver = GenerateRandomTrafficCarDriver(vehicle);
        debug_assert(carDriver);
    }

    return vehicle;
}

Pedestrian* TrafficManager::GenerateRandomTrafficPedestrian(int posx, int posy, int posz)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    // generate pedestrian
    glm::vec2 positionOffset(
        Convert::MapUnitsToMeters(random.generate_float() - 0.5f),
        Convert::MapUnitsToMeters(random.generate_float() - 0.5f)
    );
    cxx::angle_t pedestrianHeading(360.0f * random.generate_float(), cxx::angle_t::units::degrees);
    glm::vec3 pedestrianPosition(
        Convert::MapUnitsToMeters(posx + 0.5f) + positionOffset.x,
        Convert::MapUnitsToMeters(posy * 1.0f),
        Convert::MapUnitsToMeters(posz + 0.5f) + positionOffset.y
    );
    // fix height
    pedestrianPosition.y = gGameMap.GetHeightAtPosition(pedestrianPosition);

    Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pedestrianPosition, pedestrianHeading, ePedestrianType_Civilian);
    debug_assert(pedestrian);
    if (pedestrian)
    {
        pedestrian->mObjectFlags = (pedestrian->mObjectFlags | GameObjectFlags_Traffic);

        AiCharacterController* controller = gAiManager.CreateAiController(pedestrian);
        debug_assert(controller);
    }
    return pedestrian;
}

Pedestrian* TrafficManager::GenerateHareKrishnas(int posx, int posy, int posz)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    glm::vec2 positionOffset(
        Convert::MapUnitsToMeters(random.generate_float() - 0.5f),
        Convert::MapUnitsToMeters(random.generate_float() - 0.5f)
    );
    cxx::angle_t pedestrianHeading(360.0f * random.generate_float(), cxx::angle_t::units::degrees);
    glm::vec3 pedestrianPosition(
        Convert::MapUnitsToMeters(posx + 0.5f) + positionOffset.x,
        Convert::MapUnitsToMeters(posy * 1.0f),
        Convert::MapUnitsToMeters(posz + 0.5f) + positionOffset.y
    );
    // fix height
    pedestrianPosition.y = gGameMap.GetHeightAtPosition(pedestrianPosition);

    // only one group lives at once, old group from cheats goes away
    RemoveHareKrishnasGroup();
    mGenPedsSinceHareKrishnas = 0;
    mIsHareKrishnasChanting = true;

    Pedestrian* characterLeader = nullptr;
    Pedestrian* characterPrev = nullptr;
    for (int i = 0; i < gGameParams.mTrafficHareKrishnasGroupSize; ++i)
    {
        Pedestrian* character = gGameObjectsManager.CreatePedestrian(pedestrianPosition, pedestrianHeading, ePedestrianType_HareKrishnasGang);
        debug_assert(character);
        mHareKrishnasGroup.push_back(character);

        character->mObjectFlags = (character->mObjectFlags | GameObjectFlags_Traffic);
        AiCharacterController* controller = gAiManager.CreateAiController(character);
        debug_assert(controller);
        if (controller && characterLeader == nullptr)
        {
            controller->ChangeAiFlags(PedestrianAiFlags_FollowHumanCharacter, PedestrianAiFlags_None);
            characterLeader = character;
        }
        if (controller && characterPrev)
        {
            // followers only keep the line, only leader reacts on threats
            controller->ChangeAiFlags(PedestrianAiFlags_IgnoreThreats, PedestrianAiFlags_None);
            controller->FollowPedestrian(characterPrev);
        }
        characterPrev = character;
    }
    return characterLeader;
}

Pedestrian* TrafficManager::GenerateRandomTrafficCarDriver(Vehicle* car)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    debug_assert(car);

    int remapIndex = random.generate_int(0, MAX_PED_REMAPS - 1); // todo: find out correct list of traffic peds skins
    Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(car->mTransform.mPosition, car->mTransform.mOrientation, ePedestrianType_Civilian);
    debug_assert(pedestrian);

    if (pedestrian)
    {
        pedestrian->mObjectFlags = (pedestrian->mObjectFlags | GameObjectFlags_Traffic);

        AiCharacterController* controller = gAiManager.CreateAiController(pedestrian);
        debug_assert(controller);
        if (controller)
        {
            pedestrian->PutInsideCar(car, eCarSeat_Driver);
        }
    }
    return pedestrian;
}

bool TrafficManager::TryRemoveTrafficCar(Vehicle* car)
{
    if (car->IsMarkedForDeletion())
        return true;

    if (!car->IsTrafficFlag())
        return false;

    bool hasNonTrafficPassengers = cxx::contains_if(car->mPassengers, [](Pedestrian* carDriver)
    {
        return !carDriver->IsTrafficFlag();
    });

    if (hasNonTrafficPassengers)
        return false;

    for (Pedestrian* currDriver: car->mPassengers)
    {
        currDriver->MarkForDeletion();
    }

    car->MarkForDeletion();
    return true;
}

bool TrafficManager::TryRemoveTrafficPed(Pedestrian* pedestrian)
{
    if (pedestrian->IsMarkedForDeletion())
        return true;

    if (!pedestrian->IsTrafficFlag())
        return false;
        
    pedestrian->MarkForDeletion();
    return true;
}