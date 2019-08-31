#pragma once

#include "CityMeshBuilder.h"

// renders map mesh, peds, cars and map objects
class CityRenderer final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();

    void RenderFrame();

private:
    // internals
    void BuildCityMeshData();
    void CommitCityMeshData();

    void RenderFrameBegin();
    void RenderFrameEnd();

    void RenderCityMesh();
    void RenderPeds();
    void RenderCars();
    void RenderMapObjects();
    void RenderProjectiles();

private:
    // objects data
    StreamingVertexCache mSpritesVertexCache; 

    // map mesh data
    Rect2D mCityMeshArea;
    CityMeshData mCityMeshData[MAP_LAYERS_COUNT];
    CityMeshBuilder mCityMeshBuilder;

    // map mesh vertex buffers
    GpuBuffer* mCityMeshBufferV;
    GpuBuffer* mCityMeshBufferI;
};