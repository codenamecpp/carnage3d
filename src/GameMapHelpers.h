#pragma once

#include "GameDefs.h"

class GameMapManager;
class GameMapHelpers final
{
public:
    // construct mesh for specified city area and layer
    // @param cityScape: City scape data
    // @param area: Target map rect
    // @param layerIndex: Target map layer, see MAP_LAYERS_COUNT
    // @param meshData: Output mesh data
    static bool BuildMapMesh(GameMapManager& city, const Rect& area, int layerIndex, MapMeshData& meshData);
    static bool BuildMapMesh(GameMapManager& city, const Rect& area, MapMeshData& meshData);

    // compute height for specific block slope type
    // @param slopeType: Slope type
    // @param x, y: Position within block [0, 1]
    // @return slope height specified in map units [0, 1]
    static float GetSlopeHeight(int slopeType, float x, float y);

private:
    // internals
    static void PutBlockFace(GameMapManager& city, MapMeshData& meshData, int x, int y, int z, eBlockFace face, MapBlockInfo* blockInfo);
};