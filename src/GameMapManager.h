#pragma once

#include "GameDefs.h"
#include "StyleData.h"

// this class manages GTA map and style data which get loaded from CMP/G24-files
class GameMapManager final: public cxx::noncopyable
{
public:
    // readonly
    StyleData mStyleData;

    std::vector<StartupObjectPosStruct> mStartupObjects;

    // audio bank and style numbers
    int mStyleFileNumber = 0;
    int mAudioFileNumber = 0;

public:
    // load map data from specific file, returns false on error
    // @param filename: Target file name
    bool LoadFromFile(const std::string& filename);

    // free currently loaded map data
    void Cleanup();

    // test whether city scape data was loaded, including style data
    bool IsLoaded() const;

    // get map block info at specific location
    // note that location coords should never exceed MAP_DIMENSIONS for x,y and MAP_LAYERS_COUNT for layer
    // @param coordx, coordy, layer: Block location
    const MapBlockInfo* GetBlockInfo(int coordx, int coordy, int layer) const;

    // Get navigation data sector at specific map point
    // @param position: Current position on map, meters
    // @returns null on error
    const DistrictInfo* GetDistrictAtPosition2(const glm::vec2& position) const;
    const DistrictInfo* GetDistrict(int coordx, int coordy) const;
    const DistrictInfo* GetDistrictByIndex(int districtIndex) const;

    // Get real height at specified map point
    // @param position: Current position on map, meters
    float GetHeightAtPosition(const glm::vec3& position, bool excludeWater = true) const;

    // Get water height at specific map point
    // @param position: Current position on map, meters
    float GetWaterLevelAtPosition2(const glm::vec2& position) const;

    // get intersection with solid blocks on specific map layer, ignores slopes
    // @param origin: Start position
    // @param destination: End position
    // @param height: Z coord which is map layer
    // @param outPoint: Intersection point
    // @returns true if intersection detected or false otherwise
    bool TraceSegment2D(const glm::vec2& origin, const glm::vec2& destination, float height, glm::vec2& outPoint);

private:
    // Reading map data internals
    // @param file: Source stream
    bool ReadCompressedMapData(std::istream& file, int columnLength, int blockLength);
    bool ReadStartupObjects(std::istream& file, int dataSize);
    bool ReadRoutes(std::istream& file, int dataSize);
    bool ReadServiceBaseLocations(std::ifstream& file);
    bool ReadNavData(std::ifstream& file, int dataSize);
    void FixShiftedBits();

private:
    MapBlockInfo mMapTiles[MAP_LAYERS_COUNT][MAP_DIMENSIONS][MAP_DIMENSIONS]; // z, y, x
    int mBaseTilesData[MAP_DIMENSIONS][MAP_DIMENSIONS]; // y x

    // accident service base locations
    std::vector<glm::ivec3> mAccidentServicesBases[eAccidentServise_COUNT];

    std::vector<DistrictInfo> mDistricts;
};

extern GameMapManager gGameMap;