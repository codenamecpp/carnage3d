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

    void DrawCityMesh();
    void DrawPeds();
    void DrawCars();
    void DrawMapObjects();
    void DrawProjectiles();

    void DrawSprite3D();
    void DrawSprite2D();

private:
    StreamingVertexCache mSpritesVertexCache; 

    Rect2D mCityMeshMapRect; // cached map area in tiles

    CityMeshData mCityMeshData[MAP_LAYERS_COUNT];
    CityMeshBuilder mCityMeshBuilder;

    GpuBuffer* mCityMeshBufferV;
    GpuBuffer* mCityMeshBufferI;
};