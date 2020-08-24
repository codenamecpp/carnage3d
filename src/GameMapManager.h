#pragma once

#include "GameDefs.h"
#include "StyleData.h"

/*
    1 map unit == 4.0 meters (see gamedefs)

     ^^^^^^^^
    |        |
    |        |
    |________|_ _ _ _ _ _ _ _ 12.0 meters (3 map units)
     ________  
    |        |
    |        |
    |        |
    |________|_ _ _ _ _ _ _ _ 8.0 meters (2 map units)
     ________  
    |        |
    |        |
    |        |
    |________|_ _ _ _ _ _ _ _ 4.0 meters (1 map unit)
     ________  
    |        |
    |        |
    |        |
    |________|_ _ _ _ _ _ _ _ 0.0 meters (0 map units)
*/

// this class manages GTA map and style data which get loaded from CMP/G24-files
class GameMapManager final: public cxx::noncopyable
{
public:
    // public for convenience
    StyleData mStyleData;

    std::vector<StartupObjectPosStruct> mStartupObjects;

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
    MapBlockInfo* GetBlock(int coordx, int coordy, int layer) const;
    MapBlockInfo* GetBlockClamp(int coordx, int coordy, int layer) const;

    // Get real height at specified map point
    // @param position: Current position on map
    float GetHeightAtPosition(const glm::vec3& position, bool excludeWater = true) const;

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
    bool ReadCompressedMapData(std::ifstream& file, int columnLength, int blockLength);
    bool ReadStartupObjects(std::ifstream& file, int dataSize);
    void FixShiftedBits();

private:
    MapBlockInfo mMapTiles[MAP_LAYERS_COUNT][MAP_DIMENSIONS][MAP_DIMENSIONS]; // z, y, x
    int mBaseTilesData[MAP_DIMENSIONS][MAP_DIMENSIONS]; // y x
};

extern GameMapManager gGameMap;