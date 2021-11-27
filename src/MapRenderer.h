#pragma once

#include "SpriteBatch.h"
#include "GameDefs.h"

class DebugRenderer;

// map renderer statistics info
struct MapRenderStats
{
public:
    MapRenderStats() = default;
    void FrameBegin();
    void FrameEnd();

public:
    int mBlockChunksDrawnCount = 0;  // per frame
    int mSpritesDrawnCount = 0; // per frame

    unsigned int mRenderFramesCounter = 0; // gets incremented on every frame
};

// renders map mesh, peds, cars and map objects
class MapRenderer final: public cxx::noncopyable
{
public:
    MapRenderStats mRenderStats;

public:
    bool Initialize();
    void Deinit();
    void RenderFrameBegin();
    void RenderFrame(GameCamera* renderview);
    void DebugDraw(DebugRenderer& debugRender);
    void RenderFrameEnd();
    void BuildMapMesh();

private:
    void DrawCityMesh(GameCamera* renderview);
    void DrawGameObject(GameCamera* renderview, GameObject* gameObject);
    void PreDrawGameObject(GameObject* gameObject);

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

    SpriteBatch mSpriteBatch;
};