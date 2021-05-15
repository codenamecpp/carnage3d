#include "stdafx.h"
#include "TrafficManager.h"
#include "DebugRenderer.h"
#include "CarnageGame.h"
#include "RenderView.h"
#include "TimeManager.h"
#include "AiManager.h"
#include "GameCheatsWindow.h"
#include "AiCharacterController.h"

TrafficManager gTrafficManager;

TrafficManager::TrafficManager()
{
}

void TrafficManager::StartupTraffic()
{   
    mLastGenHareKrishnasTime = gTimeManager.mGameTime;

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
}

void TrafficManager::UpdateFrame()
{
    GeneratePeds();
    GenerateCars();
}

void TrafficManager::DebugDraw(DebugRenderer& debugRender)
{
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

        int generatePedsCount = GetPedsToGenerateCount(humanPlayer->mPlayerView);
        if (generatePedsCount > 0)
        {
            GenerateTrafficPeds(generatePedsCount, humanPlayer->mPlayerView);
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

        bool isOnScreen = false;
        for (HumanPlayer* humanPlayer: gCarnageGame.mHumanPlayers)
        {   
            if (humanPlayer == nullptr)
                continue;

            cxx::aabbox2d_t onScreenArea = humanPlayer->mPlayerView.mOnScreenArea;
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

void TrafficManager::GenerateTrafficPeds(int pedsCount, RenderView& view)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    int numPedsGenerated = 0;

    Rect innerRect;
    Rect outerRect;
    // get map area on screen
    {
        Point minBlock;
        minBlock.x = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMin.x);
        minBlock.y = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMin.y);

        Point maxBlock;
        maxBlock.x = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMax.x) + 1;
        maxBlock.y = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMax.y) + 1;

        innerRect.x = minBlock.x;
        innerRect.y = minBlock.y;
        innerRect.w = (maxBlock.x - minBlock.x);
        innerRect.h = (maxBlock.y - minBlock.y);

        // expand
        int expandSize = gGameParams.mTrafficGenPedsMaxDistance;
        outerRect = innerRect;
        outerRect.x -= expandSize;
        outerRect.y -= expandSize;
        outerRect.w += expandSize * 2;
        outerRect.h += expandSize * 2;
    }
    
    mCandidatePosArray.clear();

    for (int iy = 0; iy < outerRect.h; ++iy)
    for (int ix = 0; ix < outerRect.w; ++ix)
    {
        Point pos (ix + outerRect.x, iy + outerRect.y);
        if (innerRect.PointWithin(pos))
            continue;

        // scan candidate from top
        for (int iz = (MAP_LAYERS_COUNT - 1); iz > 0; --iz)
        {
            const MapBlockInfo* mapBlock = gGameMap.GetBlockInfo(pos.x, pos.y, iz);

            if (mapBlock->mGroundType == eGroundType_Air)
                continue;

            if (mapBlock->mGroundType == eGroundType_Pawement)
            {
                if (mapBlock->mIsRailway)
                    continue;

                CandidatePos candidatePos;
                candidatePos.mMapX = pos.x;
                candidatePos.mMapY = pos.y;
                candidatePos.mMapLayer = iz;
                mCandidatePosArray.push_back(candidatePos);
            }
            break;
        }
    }

    if (mCandidatePosArray.empty())
        return;

    // shuffle candidates
    random.shuffle(mCandidatePosArray);

    for (; (numPedsGenerated < pedsCount) && !mCandidatePosArray.empty(); ++numPedsGenerated)
    {
        if (!random.random_chance(gGameParams.mTrafficGenPedsChance))
            continue;

        CandidatePos candidate = mCandidatePosArray.back();
        mCandidatePosArray.pop_back();

        if ((mLastGenHareKrishnasTime + gGameParams.mTrafficGenHareKrishnasTime) < gTimeManager.mGameTime)
        {
            // generate hare krishnas
            GenerateHareKrishnas(candidate.mMapX, candidate.mMapLayer, candidate.mMapY);
            mLastGenHareKrishnasTime = gTimeManager.mGameTime;
        }
        else
        {
            // generate normal pedestrian
            GenerateRandomTrafficPedestrian(candidate.mMapX, candidate.mMapLayer, candidate.mMapY);
        }
    }
}

int TrafficManager::GetPedsToGenerateCount(RenderView& view) const
{
    int pedestriansCounter = 0;

    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenPedsMaxDistance * 1.0f);

    cxx::aabbox2d_t onScreenArea = view.mOnScreenArea;
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

int TrafficManager::GetCarsToGenerateCount(RenderView& view) const
{
    int carsCounter = 0;

    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenCarsMaxDistance * 1.0f);

    cxx::aabbox2d_t onScreenArea = view.mOnScreenArea;
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

        int generateCarsCount = GetCarsToGenerateCount(humanPlayer->mPlayerView);
        if (generateCarsCount > 0)
        {
            GenerateTrafficCars(generateCarsCount, humanPlayer->mPlayerView);
        }
    }
}

void TrafficManager::GenerateTrafficCars(int carsCount, RenderView& view)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    int numCarsGenerated = 0;

    Rect innerRect;
    Rect outerRect;
    // get map area on screen
    {
        Point minBlock;
        minBlock.x = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMin.x);
        minBlock.y = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMin.y);

        Point maxBlock;
        maxBlock.x = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMax.x) + 1;
        maxBlock.y = (int) Convert::MetersToMapUnits(view.mOnScreenArea.mMax.y) + 1;

        innerRect.x = minBlock.x;
        innerRect.y = minBlock.y;
        innerRect.w = (maxBlock.x - minBlock.x);
        innerRect.h = (maxBlock.y - minBlock.y);

        // expand
        int expandSize = gGameParams.mTrafficGenCarsMaxDistance;
        outerRect = innerRect;
        outerRect.x -= expandSize;
        outerRect.y -= expandSize;
        outerRect.w += expandSize * 2;
        outerRect.h += expandSize * 2;
    }
    
    mCandidatePosArray.clear();

    for (int iy = 0; iy < outerRect.h; ++iy)
    for (int ix = 0; ix < outerRect.w; ++ix)
    {
        Point pos (ix + outerRect.x, iy + outerRect.y);
        if (innerRect.PointWithin(pos))
            continue;

        // scan candidate from top
        for (int iz = (MAP_LAYERS_COUNT - 1); iz > 0; --iz)
        {
            const MapBlockInfo* mapBlock = gGameMap.GetBlockInfo(pos.x, pos.y, iz);

            if (mapBlock->mGroundType == eGroundType_Air)
                continue;

            if (mapBlock->mGroundType == eGroundType_Road)
            {
                int bits = (int) (mapBlock->mDownDirection) + 
                    (int) (mapBlock->mUpDirection) +
                    (int) (mapBlock->mLeftDirection) + 
                    (int) (mapBlock->mRightDirection);

                if ((bits == 0 || bits > 1) || mapBlock->mIsRailway)
                    continue;

                CandidatePos candidatePos;
                candidatePos.mMapX = pos.x;
                candidatePos.mMapY = pos.y;
                candidatePos.mMapLayer = iz;
                mCandidatePosArray.push_back(candidatePos);
            }
            break;
        }
    }

    if (mCandidatePosArray.empty())
        return;

    // shuffle candidates
    random.shuffle(mCandidatePosArray);

    for (; (numCarsGenerated < carsCount) && !mCandidatePosArray.empty(); ++numCarsGenerated)
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

            cxx::aabbox2d_t onScreenArea = humanPlayer->mPlayerView.mOnScreenArea;
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

    Pedestrian* characterLeader = nullptr;
    Pedestrian* characterPrev = nullptr;
    for (int i = 0, PedsCount = 7; i < PedsCount; ++i)
    {
        Pedestrian* character = gGameObjectsManager.CreatePedestrian(pedestrianPosition, pedestrianHeading, ePedestrianType_HareKrishnasGang);
        debug_assert(character);

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