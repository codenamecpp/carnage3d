#pragma once

// defines city mesh builder class
class CityMeshBuilder final
{
public:
    // construct mesh for specified city area and layer
    // @param cityScape: City scape data
    // @param area: Target map rect
    // @param layerIndex: Target map layer, see MAP_LAYERS_COUNT
    // @param meshData: Output mesh data
    bool Build(CityScapeData& cityScape, const Rect2D& area, int layerIndex, CityMeshData& meshData);
    bool Build(CityScapeData& cityScape, const Rect2D& area, CityMeshData& meshData);

private:
    // internals
    void PutBlockFace(CityMeshData& meshData, int posx, int posy, int posz, eBlockFace face, BlockStyleData* blockInfo);
};