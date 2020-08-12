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
    // @param slope: Index
    // @param pxcoord_x, pxcoord_y: Position within block specified in pixels [0, MAP_PIXELS_PER_TILE)
    // @return slope height specified in pixels [0, MAP_PIXELS_PER_TILE]
    static int GetSlopeHeight(int slope, int pxcoord_x, int pxcoord_y);

private:
    GameMapHelpers();
    // internals
    static void PutBlockFace(GameMapManager& city, MapMeshData& meshData, int x, int y, int z, eBlockFace face, BlockStyle* blockInfo);
};