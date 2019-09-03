#pragma once

#include "GameDefs.h"

// defines part of city mesh
struct CityBlocksMeshData
{
public:
    CityBlocksMeshData() = default;
    inline void SetNull()
    {
        mMeshVertices.clear();
        mMeshIndices.clear();
    }
public:
    std::vector<CityVertex3D> mMeshVertices;
    std::vector<DrawIndex_t> mMeshIndices;
};

// defines city mesh builder class
class CityMeshBuilder final
{
public:
    // construct mesh for specified city area and layer
    // @param cityScape: City scape data
    // @param area: Target map rect
    // @param layerIndex: Target map layer, see MAP_LAYERS_COUNT
    // @param meshData: Output mesh data
    bool Build(CityScapeData& cityScape, const Rect2D& area, int layerIndex, CityBlocksMeshData& meshData);
    bool Build(CityScapeData& cityScape, const Rect2D& area, CityBlocksMeshData& meshData);

private:
    // internals
    void PutBlockFace(CityScapeData& cityScape, CityBlocksMeshData& meshData, int posx, int posy, int posz, eBlockFace face, BlockStyleData* blockInfo);
};