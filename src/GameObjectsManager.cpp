#include "stdafx.h"
#include "GameObjectsManager.h"
#include "Vehicle.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"
#include "Projectile.h"
#include "RenderingManager.h"

GameObjectsManager gGameObjectsManager;

GameObjectsManager::~GameObjectsManager()
{
    mPedestriansPool.cleanup();
    mCarsPool.cleanup();
    mProjectilesPool.cleanup();
    mDecorationsPool.cleanup();
    mObstaclesPool.cleanup();
    mExplosionsPool.cleanup();
}

bool GameObjectsManager::InitGameObjects()
{
    mIDsCounter = 0;

    if (!CreateStartupObjects())
    {
        gConsole.LogMessage(eLogMessage_Warning, "GameObjectsManager: Cannot create startup objects");
        return false;
    }

    return true;
}

void GameObjectsManager::FreeGameObjects()
{
    DestroyAllObjects();
}

void GameObjectsManager::UpdateFrame()
{
    DestroyMarkedForDeletionObjects();

    // if is safe to add new objects during loop by adding them to the end of the list

    const size_t allObjectsCount = mAllObjectsList.size();
    for (size_t iobject = 0; iobject < allObjectsCount; ++iobject)
    {
        GameObject* currentObject = mAllObjectsList[iobject];
        currentObject->UpdateFrame();
    }
}

void GameObjectsManager::DebugDraw()
{
}

Pedestrian* GameObjectsManager::CreatePedestrian(const glm::vec3& position, cxx::angle_t heading)
{
    GameObjectID pedestrianID = GenerateUniqueID();

    Pedestrian* instance = mPedestriansPool.create(pedestrianID);
    debug_assert(instance);

    mAllObjectsList.push_back(instance);

    // init
    instance->Spawn(position, heading);
    return instance;
}

Vehicle* GameObjectsManager::CreateCar(const glm::vec3& position, cxx::angle_t heading, CarStyle* carStyle)
{
    debug_assert(gGameMap.mStyleData.IsLoaded());
    debug_assert(carStyle);
    GameObjectID carID = GenerateUniqueID();

    Vehicle* instance = mCarsPool.create(carID);
    debug_assert(instance);

    mAllObjectsList.push_back(instance);

    // init
    instance->mCarStyle = carStyle;
    instance->Spawn(position, heading);
    return instance;
}

Vehicle* GameObjectsManager::CreateCar(const glm::vec3& position, cxx::angle_t heading, eCarModel carModel)
{
    Vehicle* vehicle = nullptr;
    for (CarStyle& currStyle: gGameMap.mStyleData.mCars)
    {
        if (currStyle.mModelId == carModel)
        {
            vehicle = CreateCar(position, heading, &currStyle);
            break;
        }
    }
    debug_assert(vehicle);
    return vehicle;
}

Projectile* GameObjectsManager::CreateProjectile(const glm::vec3& position, cxx::angle_t heading)
{
    return CreateProjectile(position, heading, nullptr);
}

Projectile* GameObjectsManager::CreateProjectile(const glm::vec3& position, cxx::angle_t heading, WeaponInfo* weaponInfo)
{
    Projectile* instance = mProjectilesPool.create();
    debug_assert(instance);

    mAllObjectsList.push_back(instance);
    // init
    instance->Spawn(position, heading, weaponInfo);
    return instance;
}

Obstacle* GameObjectsManager::CreateObstacle(const glm::vec3& position, cxx::angle_t heading, GameObjectStyle* desc)
{
    Obstacle* instance = nullptr;
    debug_assert(gGameMap.mStyleData.IsLoaded());
    debug_assert(desc);
    debug_assert(desc->mClassID == eGameObjectClass_Obstacle);
    if (desc->mClassID == eGameObjectClass_Obstacle)
    {
        GameObjectID objectID = GenerateUniqueID();

        instance = mObstaclesPool.create(objectID, desc);
        debug_assert(instance);

        mAllObjectsList.push_back(instance);
        // init
        instance->Spawn(position, heading);
    }
    return instance;
}

Explosion* GameObjectsManager::CreateExplosion(const glm::vec3& position)
{
    GameObjectID objectID = GenerateUniqueID();

    Explosion* instance = mExplosionsPool.create(objectID);
    debug_assert(instance);

    mAllObjectsList.push_back(instance);
    // init
    instance->Spawn(position);
    return instance;
}

Decoration* GameObjectsManager::CreateDecoration(const glm::vec3& position, cxx::angle_t heading, GameObjectStyle* desc)
{
    Decoration* instance = nullptr;
    debug_assert(gGameMap.mStyleData.IsLoaded());
    debug_assert(desc);
    debug_assert(desc->mClassID == eGameObjectClass_Decoration);
    if (desc->mClassID == eGameObjectClass_Decoration)
    {
        GameObjectID objectID = GenerateUniqueID();

        instance = mDecorationsPool.create(objectID, desc);
        debug_assert(instance);

        mAllObjectsList.push_back(instance);
        // init
        instance->Spawn(position, heading);
    }
    return instance;
}

Obstacle* GameObjectsManager::GetObstacleByID(GameObjectID objectID) const
{
    for (GameObject* currentObject: mAllObjectsList)
    {
        if (currentObject->mObjectID != objectID)
            continue;

        if (currentObject->IsMarkedForDeletion() || !currentObject->IsObstacleClass())
            return nullptr;

        return static_cast<Obstacle*>(currentObject);
    }
    return nullptr;
}

Vehicle* GameObjectsManager::GetVehicleByID(GameObjectID objectID) const
{
    for (GameObject* currentObject: mAllObjectsList)
    {
        if (currentObject->mObjectID != objectID)
            continue;

        if (currentObject->IsMarkedForDeletion() || !currentObject->IsVehicleClass())
            return nullptr;

        return static_cast<Vehicle*>(currentObject);
    }
    return nullptr;
}

Decoration* GameObjectsManager::GetDecorationByID(GameObjectID objectID) const
{
    for (GameObject* currentObject: mAllObjectsList)
    {
        if (currentObject->mObjectID != objectID)
            continue;

        if (currentObject->IsMarkedForDeletion() || !currentObject->IsDecorationClass())
            return nullptr;

        return static_cast<Decoration*>(currentObject);
    }
    return nullptr;
}

Pedestrian* GameObjectsManager::GetPedestrianByID(GameObjectID objectID) const
{
    for (GameObject* currentObject: mAllObjectsList)
    {
        if (currentObject->mObjectID != objectID)
            continue;

        if (currentObject->IsMarkedForDeletion() || !currentObject->IsPedestrianClass())
            return nullptr;

        return static_cast<Pedestrian*>(currentObject);
    }
    return nullptr;
}

GameObject* GameObjectsManager::GetGameObjectByID(GameObjectID objectID) const
{
    for (GameObject* currentObject: mAllObjectsList)
    {
        if (currentObject->mObjectID != objectID)
            continue;

        if (currentObject->IsMarkedForDeletion())
            return nullptr;

        return currentObject;
    }
    return nullptr;
}

void GameObjectsManager::MarkForDeletion(GameObject* object)
{
    if (object->IsMarkedForDeletion())
        return;

    mDeleteObjectsList.push_back(object);
    object->mMarkedForDeletion = true;
}

void GameObjectsManager::DestroyGameObject(GameObject* object)
{
    if (object == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::erase_elements(mDeleteObjectsList, object);
    cxx::erase_elements(mAllObjectsList, object);

    switch (object->mObjectTypeID)
    {
        case eGameObjectClass_Pedestrian:
        {
            Pedestrian* pedestrian = static_cast<Pedestrian*>(object);
            mPedestriansPool.destroy(pedestrian);
        }
        break;

        case eGameObjectClass_Car:
        {
            Vehicle* vehicle = static_cast<Vehicle*>(object);
            mCarsPool.destroy(vehicle);
        }
        break;

        case eGameObjectClass_Projectile:
        {
            Projectile* projectile = static_cast<Projectile*>(object);
            mProjectilesPool.destroy(projectile);
        }
        break;

        case eGameObjectClass_Decoration:
        {
            Decoration* decoration = static_cast<Decoration*>(object);
            mDecorationsPool.destroy(decoration);
        }
        break;

        case eGameObjectClass_Obstacle:
        {
            Obstacle* obstacle = static_cast<Obstacle*>(object);
            mObstaclesPool.destroy(obstacle);
        }
        break;

        case eGameObjectClass_Explosion:
        {
            Explosion* explosion = static_cast<Explosion*>(object);
            mExplosionsPool.destroy(explosion);
        }
        break;

        case eGameObjectClass_Powerup:
        default:
        {
            debug_assert(false);
        }
    }
}

void GameObjectsManager::DestroyAllObjects()
{
    while (!mAllObjectsList.empty())
    {
        DestroyGameObject(mAllObjectsList[0]);
    }
}

void GameObjectsManager::DestroyMarkedForDeletionObjects()
{
    while (!mDeleteObjectsList.empty())
    {
        DestroyGameObject(mDeleteObjectsList[0]);
    }
}

GameObjectID GameObjectsManager::GenerateUniqueID()
{
    GameObjectID newID = ++mIDsCounter;
    if (newID == GAMEOBJECT_ID_NULL) // overflow
    {
        debug_assert(false);
    }
    return newID;
}

bool GameObjectsManager::CreateStartupObjects()
{
    debug_assert(gGameMap.IsLoaded());

    StyleData& styleData = gGameMap.mStyleData;
    for (const StartupObjectPosStruct& currObject: gGameMap.mStartupObjects)
    {
        int mapLevel = (int) Convert::PixelsToMapUnits(currObject.mZ);
        mapLevel = INVERT_MAP_LAYER(mapLevel);
        glm::vec3 start_position 
        { 
            Convert::PixelsToMeters(currObject.mX),
            Convert::MapUnitsToMeters(mapLevel * 1.0f),
            Convert::PixelsToMeters(currObject.mY) 
        };

        cxx::angle_t start_rotation = Convert::Fix16ToAngle(currObject.mRotation);

        // create startup cars
        if (currObject.IsCarObject())
        {
            eCarModel carModel;
            if (!cxx::parse_enum_int(currObject.mType, carModel))
            {
                debug_assert(false);
                continue;
            }
            Vehicle* startupCar = CreateCar(start_position, start_rotation, carModel);
            debug_assert(startupCar);
            continue;
        }

        int objectTypeIndex = currObject.mType;

        debug_assert(objectTypeIndex < GameObjectType_MAX);

        GameObjectStyle& objectType = styleData.mGameObjects[objectTypeIndex];
        switch (objectType.mClassID)
        {
            case eGameObjectClass_Decoration: 
            {
                Decoration* startupDecoration = CreateDecoration(start_position, start_rotation, &objectType);
                debug_assert(startupDecoration);
            }
            break;

            case eGameObjectClass_Obstacle: 
            {
                Obstacle* startupObstacle = CreateObstacle(start_position, start_rotation, &objectType);
                debug_assert(startupObstacle);
            }            
            break;

            case eGameObjectClass_Powerup: break;

            default:
                debug_assert(false);
            break;
        }
    }
    return true;
}

int GameObjectsManager::GetBaseHitpointsForCar(eCarVType carType) const
{
    // todo: move to settings
    // todo: find out correct values

    switch (carType)
    {
        case eCarVType_Bus:
        case eCarVType_FrontOfJuggernaut:
        case eCarVType_BackOfJuggernaut:
            return 29; 
        case eCarVType_Motorcycle:
        case eCarVType_StandardCar:
            return 16;
        case eCarVType_Train:
        case eCarVType_Tank:
            return 62;
    }
    return 10;
}
