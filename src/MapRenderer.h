#pragma once

#include "SpriteBatch.h"
#include "GameDefs.h"

class RenderView;

// map renderer statistics info
struct MapRenderStats
{
public:
    MapRenderStats() = default;
    void FrameStart();
    void FrameEnd();

public:
    int mBlockChunksDrawnCount = 0; // per frame
};

// renders map mesh, peds, cars and map objects
class MapRenderer final: public cxx::noncopyable
{
public:
    MapRenderStats mRenderStats;

public:
    bool Initialize();
    void Deinit();
    void RenderFrameStart();
    void RenderFrame(RenderView* renderview);
    void RenderFrameEnd();
    void BuildMapMesh();

private:
    void DrawCityMesh(RenderView* renderview);

private:
    enum
    {
        BlocksBatchDims = 22, // 22 x 22 x 6 blocks per batch
        ExtraBlocksPerSide = 4,
        BlocksBatchesPerSide = ((MAP_DIMENSIONS + (ExtraBlocksPerSide * 2)) + BlocksBatchDims - 1) / BlocksBatchDims,
        BlocksBatchCount = BlocksBatchesPerSide * BlocksBatchesPerSide,
    };
    struct MapBlocksChunk
    {
        cxx::aabbox_t mBounds; // for culling
        // index/vertex data offset in vbo
        unsigned int mIndicesStart = 0, mIndicesCount = 0;
        unsigned int mVerticesStart = 0, mVerticesCount = 0;
    };
    MapBlocksChunk mMapBlocksChunks[BlocksBatchCount];

    GpuBuffer* mCityMeshBufferV;
    GpuBuffer* mCityMeshBufferI;

    SpriteBatch mSpritesBatch;
};