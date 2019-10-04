#pragma once

#include "SpriteBatch.h"

// renders map mesh, peds, cars and map objects
class MapRenderer final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    void RenderFrame();
    void InvalidateMapMesh();

private:
    void BuildMapMesh();
    void CommitCityMeshData();
    void DrawCityMesh();

private:
    Rect2D mCityMapRectangle; // cached map area in tiles
    SpriteBatch mSpritesBatch;
    MapMeshData mCityMeshData[MAP_LAYERS_COUNT];
    GpuBuffer* mCityMeshBufferV;
    GpuBuffer* mCityMeshBufferI;
};