#include "stdafx.h"
#include "GameObjectsManager.h"
#include "Vehicle.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"

GameObjectsManager gGameObjectsManager;

GameObjectsManager::~GameObjectsManager()
{
    mPedestriansPool.cleanup();
    mCarsPool.cleanup();
}

bool GameObjectsManager::Initialize()
{
    mIDsCounter = 0;

    if (!CreateStartupObjects())
    {
        gConsole.LogMessage(eLogMessage_Warning, "GameObjectsManager: Cannot create startup objects");
        return false;
    }

    return true;
}

void GameObjectsManager::Deinit()
{
    DestroyObjectsInList(mDeleteList);
    DestroyObjectsInList(mObjectsList);

    debug_assert(!mCarsList.has_elements());
    debug_assert(!mPedestriansList.has_elements());
}

void GameObjectsManager::UpdateFrame(Timespan deltaTime)
{
    DestroyPendingObjects();
    
    // update pedestrians
    for (Pedestrian* currentPed: mPedestriansList) // warning: dont add or remove peds during this loop
    {
        if (mDeleteList.contains(&currentPed->mDeleteObjectsNode))
            continue;

        currentPed->UpdateFrame(deltaTime);
    }

    // update cars    
    for (Vehicle* currentCar: mCarsList) // warning: dont add or remove cars during this loop
    {
        if (mDeleteList.contains(&currentCar->mDeleteObjectsNode))
            continue;

        currentCar->UpdateFrame(deltaTime);
    }
}

void GameObjectsManager::DebugDraw()
{
}

Pedestrian* GameObjectsManager::CreatePedestrian(const glm::vec3& startPosition, cxx::angle_t startRotation)
{
    GameObjectID pedestrianID = GenerateUniqueID();

    Pedestrian* instance = mPedestriansPool.create(pedestrianID);
    debug_assert(instance);

    mPedestriansList.insert(&instance->mPedsListNode);
    mObjectsList.insert(&instance->mObjectsNode);

    // init
    instance->Spawn(startPosition, startRotation);
    return instance;
}

Vehicle* GameObjectsManager::CreateCar(const glm::vec3& startPosition, cxx::angle_t carRotation, CarStyle* carStyle)
{
    StyleData& styleData = gGameMap.mStyleData;

    debug_assert(styleData.IsLoaded());
    GameObjectID carID = GenerateUniqueID();

    Vehicle* instance = mCarsPool.create(carID);
    debug_assert(instance);

    mCarsList.insert(&instance->mCarsListNode);
    mObjectsList.insert(&instance->mObjectsNode);

    // init
    instance->mCarStyle = carStyle;
    instance->Spawn(startPosition, carRotation);
    return instance;
}

Vehicle* GameObjectsManager::CreateCar(const glm::vec3& position, cxx::angle_t carRotation, eCarModel carModel)
{
    StyleData& styleData = gGameMap.mStyleData;
    debug_assert(styleData.IsLoaded());

    for (CarStyle& currStyle: styleData.mCars)
    {
        if (currStyle.mModelId == carModel)
            return CreateCar(position, carRotation, &currStyle);
    }
    debug_assert(false);
    return nullptr;
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
            Vehicle* car = static_cast<Vehicle*>(object);

            mCarsList.remove(&car->mCarsListNode);
            mCarsPool.destroy(car);
        }
        break;

        case eGameObjectType_Projectile:
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
            int mapLevel = ConvertMapLevel(ConvertPixelsToTilePos(currObject.mZ));
            glm::vec3 carPosition 
            { 
                ConvertPixelsToMap(currObject.mX),
                mapLevel * 1.0f,
                ConvertPixelsToMap(currObject.mY) 
            };

            float rotationDegrees = ConvertFixAngleToDegs(currObject.mRotation);

            Vehicle* startupCar = CreateCar(carPosition, cxx::angle_t::from_degrees(rotationDegrees), carModel);
            debug_assert(startupCar);

            ++numCars;
        }
    }
    return true;
}
