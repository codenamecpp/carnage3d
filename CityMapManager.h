#pragma once

#include "GameDefs.h"
#include "CityStyleData.h"

// this class manages GTA map and style data which get loaded from CMP/G24-files
class CityMapManager final: public cxx::noncopyable
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
    // note that location coords should never exceed MAP_DIMENSIONS for x,y and MAP_LAYERS_COUNT for z
    // point, tilex, tiley, tilez: Location
    BlockStyleData* GetBlock(int tilex, int tiley, int tilez);
    BlockStyleData* GetBlockClamp(int tilex, int tiley, int tilez);

private:
    // Reading map data internals
    // @param file: Source stream
    bool ReadCompressedMapData(std::ifstream& file, int columnLength, int blockLength);
    void FixShiftedBits();

private:
    int mMapTiles[MAP_LAYERS_COUNT][MAP_DIMENSIONS][MAP_DIMENSIONS]; // z, y, x
    int mBaseTilesData[MAP_DIMENSIONS][MAP_DIMENSIONS]; // y x
    std::vector<BlockStyleData> mBlocksData;
};

extern CityMapManager gMapManager;