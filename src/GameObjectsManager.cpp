#include "stdafx.h"
#include "GameObjectsManager.h"
#include "Vehicle.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"
#include "Projectile.h"

GameObjectsManager gGameObjectsManager;

GameObjectsManager::~GameObjectsManager()
{
    mPedestriansPool.cleanup();
    mCarsPool.cleanup();
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
    DestroyObjectsInList(mDeleteList);
    DestroyObjectsInList(mObjectsList);

    debug_assert(!mCarsList.has_elements());
    debug_assert(!mPedestriansList.has_elements());
}

void GameObjectsManager::UpdateFrame()
{
    DestroyPendingObjects();
    
    // update pedestrians
    for (Pedestrian* currObject: mPedestriansList) // warning: dont add or remove peds during this loop
    {
        if (mDeleteList.contains(&currObject->mDeleteObjectsNode))
            continue;

        currObject->UpdateFrame();
    }

    // update cars    
    for (Vehicle* currObject: mCarsList) // warning: dont add or remove cars during this loop
    {
        if (mDeleteList.contains(&currObject->mDeleteObjectsNode))
            continue;

        currObject->UpdateFrame();
    }

    // update projectiles    
    for (Projectile* currObject: mProjectilesList) // warning: dont add or remove cars during this loop
    {
        if (mDeleteList.contains(&currObject->mDeleteObjectsNode))
            continue;

        currObject->UpdateFrame();
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

    mPedestriansList.insert(&instance->mPedsListNode);
    mObjectsList.insert(&instance->mObjectsNode);

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

    mCarsList.insert(&instance->mCarsListNode);
    mObjectsList.insert(&instance->mObjectsNode);

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

Projectile* GameObjectsManager::CreateProjectile(const glm::vec3& position, cxx::angle_t heading, eProjectileType typeID)
{
    Projectile* projectile = nullptr;
    debug_assert(typeID < eProjectileType_COUNT);
    for (ProjectileStyle& currStyle: gGameMap.mStyleData.mProjectiles)
    {
        if (currStyle.mTypeID == typeID)
        {
            projectile = CreateProjectile(position, heading, &currStyle);
            break;
        }
    }
    debug_assert(projectile);
    return projectile;
}

Projectile* GameObjectsManager::CreateProjectile(const glm::vec3& position, cxx::angle_t heading, ProjectileStyle* desc)
{
    debug_assert(gGameMap.mStyleData.IsLoaded());
    debug_assert(desc);

    Projectile* instance = mProjectilesPool.create(desc);
    debug_assert(instance);
    mObjectsList.insert(&instance->mObjectsNode);
    mProjectilesList.insert(&instance->mProjectilesListNode);
    // init
    instance->Spawn(position, heading);
    return instance;
}

Vehicle* GameObjectsManager::GetCarByID(GameObjectID objectID) const
{
    for (const Vehicle* curr: mCarsList)
    {
        if (curr->mObjectID == objectID && !mDeleteList.contains(&curr->mDeleteObjectsNode))
        {
            return const_cast<Vehicle*>(curr);
        }
    }
    return nullptr;
}

Pedestrian* GameObjectsManager::GetPedestrianByID(GameObjectID objectID) const
{
    for (const Pedestrian* curr: mPedestriansList)
    {
        if (curr->mObjectID == objectID && !mDeleteList.contains(&curr->mDeleteObjectsNode))
        {
            return const_cast<Pedestrian*>(curr);
        }
    }
    return nullptr;
}

GameObject* GameObjectsManager::GetGameObjectByID(GameObjectID objectID) const
{
    for (const GameObject* curr: mObjectsList)
    {
        if (curr->mObjectID == objectID && !mDeleteList.contains(&curr->mDeleteObjectsNode))
        {
            return const_cast<GameObject*>(curr);
        }
    }
    return nullptr;
}

void GameObjectsManager::MarkForDeletion(GameObject* object)
{
    if (!mDeleteList.contains(&object->mDeleteObjectsNode))
    {
        mDeleteList.insert(&object->mDeleteObjectsNode);
    }
}

void GameObjectsManager::DestroyGameObject(GameObject* object)
{
    if (object == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (mDeleteList.contains(&object->mDeleteObjectsNode))
    {
        mDeleteList.remove(&object->mDeleteObjectsNode);
    }

    if (mObjectsList.contains(&object->mObjectsNode))
    {
        mObjectsList.remove(&object->mObjectsNode);
    }

    switch (object->mObjectTypeID)
    {
        case eGameObjectType_Pedestrian:
        {
            Pedestrian* pedestrian = static_cast<Pedestrian*>(object);

            mPedestriansList.remove(&pedestrian->mPedsListNode);
            mPedestriansPool.destroy(pedestrian);
        }
        break;

        case eGameObjectType_Car:
        {
            Vehicle* vehicle = static_cast<Vehicle*>(object);

            mCarsList.remove(&vehicle->mCarsListNode);
            mCarsPool.destroy(vehicle);
        }
        break;

        case eGameObjectType_Projectile:
        {
            Projectile* projectile = static_cast<Projectile*>(object);

            mProjectilesList.remove(&projectile->mProjectilesListNode);
            mProjectilesPool.destroy(projectile);
        }
        break;
        case eGameObjectType_Powerup:
        case eGameObjectType_Decoration:
        case eGameObjectType_Obstacle:
        default:
        {
            debug_assert(false);
            return;
        }
    }
}

void GameObjectsManager::DestroyObjectsInList(cxx::intrusive_list<GameObject>& objectsList)
{
    while (objectsList.has_elements())
    {
        cxx::intrusive_node<GameObject>* pedestrianNode = objectsList.get_head_node();
        objectsList.remove(pedestrianNode);
        
        DestroyGameObject(pedestrianNode->get_element());
    }
}

void GameObjectsManager::DestroyPendingObjects()
{
    DestroyObjectsInList(mDeleteList);
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

    int numCars = 0;
    for (const StartupObjectPosStruct& currObject: gGameMap.mStartupObjects)
    {
        // create startup cars
        if (currObject.IsCarObject())
        {
            eCarModel carModel;
            if (!cxx::parse_enum_int(currObject.mType, carModel))
            {
                debug_assert(false);
                continue;
            }
            int mapLevel = (int) Convert::PixelsToMapUnits(currObject.mZ);
            mapLevel = INVERT_MAP_LAYER(mapLevel);
            glm::vec3 carPosition 
            { 
                Convert::PixelsToMeters(currObject.mX),
                Convert::MapUnitsToMeters(mapLevel * 1.0f),
                Convert::PixelsToMeters(currObject.mY) 
            };

            cxx::angle_t rotationDegrees = Convert::Fix16ToAngle(currObject.mRotation);

            Vehicle* startupCar = CreateCar(carPosition, rotationDegrees, carModel);
            debug_assert(startupCar);

            ++numCars;
        }
    }
    return true;
}
