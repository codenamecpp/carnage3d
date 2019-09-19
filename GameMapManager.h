#pragma once

#include "GameDefs.h"
#include "CityStyleData.h"

// this class manages GTA map and style data which get loaded from CMP/G24-files
class GameMapManager final: public cxx::noncopyable
{
public:
    // public for convenience
    CityStyleData mStyleData;

public:
    // load map data from specific file, returns false on error
    // @param filename: Target file name
    bool LoadFromFile(const char* filename);

    // free currently loaded map data
    void Cleanup();

    // test whether city scape data was loaded, including style data
    bool IsLoaded() const;

    // get map block info at specific location
    // note that location coords should never exceed MAP_DIMENSIONS for x,y and MAP_LAYERS_COUNT for layer
    // @param coordx, coordy, layer: Block location
    BlockStyleData* GetBlock(int coordx, int coordy, int layer) const;
    BlockStyleData* GetBlockClamp(int coordx, int coordy, int layer) const;

    // get real height at specified map point
    // @param position: Current position on map
    float GetHeightAtPosition(const glm::vec3& position) const;

    // get intersection with solid blocks on specific map layer
    // @param origin: Start position
    // @param destination: End position
    // @param height: Z coord which is map layer
    // @param outPoint: Intersection point
    // @returns true if intersection found or false otherwise
    bool TraceSegment2D(const glm::vec2& origin, const glm::vec2& destination, float height, glm::vec2& outPoint);

private:
    // Reading map data internals
    // @param file: Source stream
    bool ReadCompressedMapData(std::ifstream& file, int columnLength, int blockLength);
    void FixShiftedBits();

private:
    BlockStyleData mMapTiles[MAP_LAYERS_COUNT][MAP_DIMENSIONS][MAP_DIMENSIONS]; // z, y, x
    int mBaseTilesData[MAP_DIMENSIONS][MAP_DIMENSIONS]; // y x
};

extern GameMapManager gGameMap;