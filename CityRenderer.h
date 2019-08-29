#pragma once

#include "CityMeshBuilder.h"

// renders static city mesh
class CityRenderer final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    void RenderFrame();

private:
    // internals
    void BuildCityMeshData();
    void UploadVertexData();

private:
    Rect2D mCityMeshArea;
    CityMeshData mCityMeshData[MAP_LAYERS_COUNT];
    CityMeshBuilder mCityMeshBuilder;

    // vertex buffers
    GpuBuffer* mCityMeshBufferV;
    GpuBuffer* mCityMeshBufferI;
};