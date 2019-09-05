#pragma once

#include "GameDefs.h"

// defines part of city mesh
struct CityMapMeshData
{
public:
    CityMapMeshData() = default;
    inline void SetNull()
    {
        mMeshVertices.clear();
        mMeshIndices.clear();
    }
public:
    std::vector<CityVertex3D> mMeshVertices;
    std::vector<DrawIndex_t> mMeshIndices;
};

//////////////////////////////////////////////////////////////////////////

// defines city mesh builder class
class CityMapMeshBuilder final
{
public:
    // construct mesh for specified city area and layer
    // @param cityScape: City scape data
    // @param area: Target map rect
    // @param layerIndex: Target map layer, see MAP_LAYERS_COUNT
    // @param meshData: Output mesh data
    bool Build(CityMapManager& cityScape, const Rect2D& area, int layerIndex, CityMapMeshData& meshData);
    bool Build(CityMapManager& cityScape, const Rect2D& area, CityMapMeshData& meshData);

private:
    // internals
    void PutBlockFace(CityMapManager& cityScape, CityMapMeshData& meshData, int posx, int posy, int posz, eBlockFace face, BlockStyleData* blockInfo);
};