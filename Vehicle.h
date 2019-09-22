#pragma once

// defines vehicle instance
class Vehicle final: public cxx::noncopyable
{
public:
    Vehicle();
    void UpdateFrame(Timespan deltaTime);

public:
};

// defines vehicles manager class
class CarsManager final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    std::vector<Vehicle*> mActiveCarsList;
    std::vector<Vehicle*> mDestroyCarsList;

public:
    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);

    // add car instance to map at specific location
    // @param position: Real world position
    Vehicle* CreateCar(const glm::vec3& position);

    // will remove car from active list and put it to destroy list, does not destroy immediately
    // @param car: Car instance
    void RemoveCar(Vehicle* car);

private:
    void DestroyPendingCars();
    void AddToActiveList(Vehicle* car);

    unsigned int GenerateUniqueID();

private:
    unsigned int mIDsCounter;

    cxx::object_pool<Vehicle> mCarsPool;
};