#pragma once

// Pedestrian types
enum ePedestrianType
{
    ePedestrianType_Player1,
    ePedestrianType_Player2,
    ePedestrianType_Player3,
    ePedestrianType_Player4,
    ePedestrianType_Civilian,
    ePedestrianType_Police,
    ePedestrianType_HareKrishnasGang,
    ePedestrianType_Medical,
    ePedestrianType_Fireman,
    ePedestrianType_COUNT
};

decl_enum_strings(ePedestrianType);

enum ePedestrianFearFlags
{
    ePedestrianFearFlags_None = 0,
    ePedestrianFearFlags_Players = BIT(0),
    ePedestrianFearFlags_Police = BIT(1),
    ePedestrianFearFlags_GunShots = BIT(2),
    ePedestrianFearFlags_Explosions = BIT(3),
    ePedestrianFearFlags_DeadPeds = BIT(4),
};

decl_enum_as_flags(ePedestrianFearFlags);

enum ePedestrianRemapType
{
    ePedestrianRemapType_Index,
    ePedestrianRemapType_RandomCivilian,
};

// Contains properties for specific pedestrian type
struct PedestrianInfo
{
public:
    PedestrianInfo() = default;

    // Load pedestrian info from json node
    bool SetupFromConfg(cxx::json_document_node configNode);

    // Reset pedestrian info to default state
    void Clear();

    // Whether pedestrian has specific fears
    bool HasFear_Players() const
    {
        return (mFearFlags & ePedestrianFearFlags_Players) > 0;
    }
    bool HasFear_Police() const
    {
        return (mFearFlags & ePedestrianFearFlags_Police) > 0;
    }
    bool HasFear_GunShots() const
    {
        return (mFearFlags & ePedestrianFearFlags_GunShots) > 0;
    }
    bool HasFear_Explosions() const
    {
        return (mFearFlags & ePedestrianFearFlags_Explosions) > 0;
    }
    bool HasFear_DeadPeds() const
    {
        return (mFearFlags & ePedestrianFearFlags_DeadPeds) > 0;
    }

    void AddFears(ePedestrianFearFlags fearFlags);
    void RemoveFears(ePedestrianFearFlags fearFlags);

public:
    ePedestrianType mTypeID = ePedestrianType_Civilian;
    ePedestrianFearFlags mFearFlags = ePedestrianFearFlags_None;
    ePedestrianRemapType mRemapType = ePedestrianRemapType_Index;
    int mRemapIndex = NO_REMAP; // valid if ePedestrianRemapType_Index
};