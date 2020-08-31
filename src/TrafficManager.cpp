#include "stdafx.h"
#include "TrafficManager.h"
#include "DebugRenderer.h"
#include "CarnageGame.h"
#include "RenderView.h"
#include "TimeManager.h"
#include "AiManager.h"

TrafficManager gTrafficManager;

TrafficManager::TrafficManager()
{
}

void TrafficManager::StartupTraffic()
{   
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );

    mRand.set_seed((unsigned int) ms.count());

    mLastGenPedestriansTime = gTimeManager.mGameTime;
    GeneratePedestrians(false);
    
    // todo: vehicles
}

void TrafficManager::CleanupTraffic()
{
    // todo: implement
}

void TrafficManager::UpdateFrame()
{
    if (gTimeManager.mGameTime > (mLastGenPedestriansTime + mGenPedestriansCooldownTime))
    {
        mLastGenPedestriansTime = gTimeManager.mGameTime;
        GeneratePedestrians(true);
    }
    // todo: vehicles
}

void TrafficManager::DebugDraw(DebugRenderer& debugRender)
{
}

void TrafficManager::GeneratePedestrians(bool offscreenOnly)
{
    // todo: generate around each active human player

    ScanOffscreenPedestrians();

    // check limits
    if (IsTrafficPedestriansLimitReached())
        return;

    Pedestrian* character = gCarnageGame.mHumanSlot[0].mCharPedestrian;
    debug_assert(character);

    int numGenerations = 0;
    
    int minDistance = offscreenOnly ? mGenPedestriansMinDistance : 2;
    int maxDistance = offscreenOnly ? mGenPedestriansMaxDistance : 3;

    glm::vec2 charPosition = character->mPhysicsBody->GetPosition2();

    int mapX = (int) Convert::MetersToMapUnits(charPosition.x);
    int mapZ = (int) Convert::MetersToMapUnits(charPosition.y);

    Rect innerRect(mapX - minDistance, mapZ - minDistance, minDistance * 2 + 1, minDistance * 2 + 1);
    Rect outerRect(mapX - maxDistance, mapZ - maxDistance, maxDistance * 2 + 1, maxDistance * 2 + 1);

    struct CandidatePos
    {
        int mMapX;
        int mMapY;
        int mMapLayer;
    };
    std::vector<CandidatePos> candidates;

    for (int iy = 0; iy < outerRect.h; ++iy)
    for (int ix = 0; ix < outerRect.w; ++ix)
    {
        Point pos (ix + outerRect.x, iy + outerRect.y);
        if (innerRect.PointWithin(pos))
            continue;

        // scan candidate from top
        for (int iz = (MAP_LAYERS_COUNT - 1); iz > 0; --iz)
        {
            MapBlockInfo* mapBlockInfo = gGameMap.GetBlockClamp(pos.x, pos.y, iz - 1);
            debug_assert(mapBlockInfo);

            if (mapBlockInfo->mGroundType == eGroundType_Air)
                continue;

            if (mapBlockInfo->mGroundType == eGroundType_Pawement)
            {
                CandidatePos candidatePos;
                candidatePos.mMapX = pos.x;
                candidatePos.mMapY = pos.y;
                candidatePos.mMapLayer = iz;
                candidates.push_back(candidatePos);
            }
            break;
        }
    }

    if (candidates.empty())
        return;

    // shuffle candidates
    std::sort(candidates.begin(), candidates.end(), [mapX, mapZ](const CandidatePos& lhs, const CandidatePos& rhs)
        {
            // Manhattan Distance
            return (std::abs(lhs.mMapX - mapX) + std::abs(lhs.mMapY - mapZ)) >
                (std::abs(rhs.mMapX - mapX) + std::abs(rhs.mMapY - mapZ));
        });

    mRand.shuffle(candidates);

    for (; (numGenerations < mGenMaxPedestriansPerIteration) && !candidates.empty(); ++numGenerations)
    {
        if (!mRand.random_chance(mGenPedestriansChance))
            continue;

        CandidatePos candidate = candidates.back();
        candidates.pop_back();

        // generate pedestrian
        glm::vec2 positionOffset(
            Convert::MapUnitsToMeters(mRand.generate_float() - 0.5f),
            Convert::MapUnitsToMeters(mRand.generate_float() - 0.5f)
        );
        cxx::angle_t pedestrianHeading(360.0f * mRand.generate_float(), cxx::angle_t::units::degrees);
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
            pedestrian->mRemapIndex = mRand.generate_int(0, MAX_PED_REMAPS - 1); // todo: find out correct list of traffic peds skins
            pedestrian->mFlags = (pedestrian->mFlags | eGameObjectFlags_Traffic);

            AiCharacterController* controller = gAiManager.CreateAiController(pedestrian);
        }
    }
}

bool TrafficManager::IsTrafficPedestriansLimitReached() const
{
    int currentTrafficPedestriansCount = 0;
    for (Pedestrian* pedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (pedestrian->IsTrafficFlag())
        {
            ++currentTrafficPedestriansCount;
        }
    }

    return (currentTrafficPedestriansCount >= mGenMaxPedestrians);
}

void TrafficManager::ScanOffscreenPedestrians()
{
    Pedestrian* character = gCarnageGame.mHumanSlot[0].mCharPedestrian;
    glm::vec2 characterPos = character->mPhysicsBody->GetPosition2();

    for (Pedestrian* pedestrian: gGameObjectsManager.mPedestriansList)
    {
        if (pedestrian->IsMarkedForDeletion())
            continue;

        if (pedestrian->IsTrafficFlag())
        {
            // todo: check for each player
            glm::vec2 currPedestrianPos = pedestrian->mPhysicsBody->GetPosition2();

            if (glm::distance2(characterPos, currPedestrianPos) > 1800.0f) // todo: magic numbers
            {
                pedestrian->mController->Deactivate();
                pedestrian->MarkForDeletion();
            }

        }
    }
}

