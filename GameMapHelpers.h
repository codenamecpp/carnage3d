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
    static bool BuildMapMesh(GameMapManager& city, const Rect2D& area, int layerIndex, MapMeshData& meshData);
    static bool BuildMapMesh(GameMapManager& city, const Rect2D& area, MapMeshData& meshData);

    // compute height for specific block slope type
    // @param slope: Index
    // @param posx, posy: Position within block in range [0, 1]
    static float GetSlopeHeight(int slope, float posx, float posy);
    static float GetSlopeHeightMin(int slope);
    static float GetSlopeHeightMax(int slope);

private:
    GameMapHelpers();
    // internals
    static void PutBlockFace(GameMapManager& city, MapMeshData& meshData, int x, int y, int z, eBlockFace face, BlockStyle* blockInfo);
};