#include "stdafx.h"
#include "TrafficManager.h"
#include "DebugRenderer.h"
#include "CarnageGame.h"
#include "RenderView.h"
#include "TimeManager.h"
#include "AiManager.h"
#include "GameCheatsWindow.h"

TrafficManager gTrafficManager;

TrafficManager::TrafficManager()
{
}

void TrafficManager::StartupTraffic()
{   
    mLastGenPedestriansTime = 0.0f;
    GeneratePedestrians();
    
    // todo: vehicles
}

void TrafficManager::CleanupTraffic()
{
    // todo: implement
}

void TrafficManager::UpdateFrame()
{
    GeneratePedestrians();

    // todo: vehicles
}

void TrafficManager::DebugDraw(DebugRenderer& debugRender)
{
}

void TrafficManager::GeneratePedestrians()
{
    if (mLastGenPedestriansTime > 0.0f && 
        (mLastGenPedestriansTime + gGameParams.mTrafficGenPedsCooldownTime) > gTimeManager.mGameTime)
    {
        return;
    }

    mLastGenPedestriansTime = gTimeManager.mGameTime;

    RemoveOffscreenPedestrians();

    if (!gGameCheatsWindow.mEnableTrafficPedsGeneration)
        return;

    for (auto& currentSlot: gCarnageGame.mHumanSlot)
    {   
        if (currentSlot.mCharPedestrian == nullptr)
            continue;

        int generatePedsCount = GetPedestriansToGenerateCount(currentSlot.mCharView);
        if (generatePedsCount > 0)
        {
            GenerateTrafficPedestrians(generatePedsCount, currentSlot.mCharView);
        }
    }
}

void TrafficManager::RemoveOffscreenPedestrians()
{
    float offscreenDistance = Convert::MapUnitsToMeters(gGameParams.mTrafficGenPedsMaxDistance * 1.0f);

    for (Pedestrian* pedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (pedestrian->IsMarkedForDeletion() || !pedestrian->IsTrafficFlag())
            continue;

        bool isOnScreen = false;
        for (auto& currentSlot: gCarnageGame.mHumanSlot)
        {   
            if (currentSlot.mCharPedestrian == nullptr)
                continue;

            cxx::aabbox2d_t onScreenArea = currentSlot.mCharView.mOnScreenArea;
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
        if (pedestrian->mController)
        {
            pedestrian->mController->Deactivate();
        }
        pedestrian->MarkForDeletion();
    }
}

void TrafficManager::GenerateTrafficPedestrians(int pedsCount, RenderView& view)
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
    
    mCandidatePedsPosArray.clear();

    for (int iy = 0; iy < outerRect.h; ++iy)
    for (int ix = 0; ix < outerRect.w; ++ix)
    {
        Point pos (ix + outerRect.x, iy + outerRect.y);
        if (innerRect.PointWithin(pos))
            continue;

        // scan candidate from top
        for (int iz = (MAP_LAYERS_COUNT - 1); iz > 0; --iz)
        {
            MapBlockInfo* mapBlockInfo = gGameMap.GetBlockClamp(pos.x, pos.y, iz);
            debug_assert(mapBlockInfo);

            if (mapBlockInfo->mGroundType == eGroundType_Air)
                continue;

            if (mapBlockInfo->mGroundType == eGroundType_Pawement)
            {
                CandidatePedestrianPos candidatePos;
                candidatePos.mMapX = pos.x;
                candidatePos.mMapY = pos.y;
                candidatePos.mMapLayer = iz;
                mCandidatePedsPosArray.push_back(candidatePos);
            }
            break;
        }
    }

    if (mCandidatePedsPosArray.empty())
        return;

    // shuffle candidates
    random.shuffle(mCandidatePedsPosArray);

    for (; (numPedsGenerated < pedsCount) && !mCandidatePedsPosArray.empty(); ++numPedsGenerated)
    {
        if (!random.random_chance(gGameParams.mTrafficGenPedsChance))
            continue;

        CandidatePedestrianPos candidate = mCandidatePedsPosArray.back();
        mCandidatePedsPosArray.pop_back();

        // generate pedestrian
        glm::vec2 positionOffset(
            Convert::MapUnitsToMeters(random.generate_float() - 0.5f),
            Convert::MapUnitsToMeters(random.generate_float() - 0.5f)
        );
        cxx::angle_t pedestrianHeading(360.0f * random.generate_float(), cxx::angle_t::units::degrees);
        glm::vec3 pedestrianPosition(
            Convert::MapUnitsToMeters(candidate.mMapX + 0.5f) + positionOffset.x,
            Convert::MapUnitsToMeters(candidate.mMapLayer * 1.0f),
            Convert::MapUnitsToMeters(candidate.mMapY + 0.5f) + positionOffset.y
        );
        // fix height
        pedestrianPosition.y = gGameMap.GetHeightAtPosition(pedestrianPosition);

        Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pedestrianPosition, pedestrianHeading);
        debug_assert(pedestrian);
        if (pedestrian)
        {
            pedestrian->mRemapIndex = random.generate_int(0, MAX_PED_REMAPS - 1); // todo: find out correct list of traffic peds skins
            pedestrian->mFlags = (pedestrian->mFlags | eGameObjectFlags_Traffic);

            AiCharacterController* controller = gAiManager.CreateAiController(pedestrian);
        }
    }
}

int TrafficManager::GetPedestriansToGenerateCount(RenderView& view) const
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
        if (!pedestrian->IsTrafficFlag() || pedestrian->IsMarkedForDeletion())
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
    for (Pedestrian* currPedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (currPedestrian->IsMarkedForDeletion() || !currPedestrian->IsTrafficFlag())
            continue;

        ++counter;
    }
    return counter;
}
