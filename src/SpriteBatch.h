#pragma once

#include "GameDefs.h"
#include "TrimeshBuffer.h"
#include "Sprite2D.h"

// defines renderer class for 2d sprites
class SpriteBatch final: public cxx::noncopyable
{
public:

    enum DepthAxis { DepthAxis_Y, DepthAxis_Z };

    // init/deinit internal resources of sprite batch
    bool Initialize();
    void Deinit();

    void BeginBatch(DepthAxis depthAxis);

    // render all batched sprites
    void Flush();

    // discard all batched sprites
    void Clear();

    // add sprite to batch but does not draw it immediately
    // @param sourceSprite: Source sprite data
    void DrawSprite(const Sprite2D& sourceSprite);

private:
    void GenerateSpritesBatches();
    void RenderSpritesBatches();

private:
    // single batch of drawing sprites
    struct DrawSpriteBatch
    {
        unsigned int mFirstVertex;
        unsigned int mFirstIndex;
        unsigned int mVertexCount;
        unsigned int mIndexCount;
        GpuTexture2D* mSpriteTexture;
    };
    // all sprites stored as is until they needs to be flushed
    std::vector<Sprite2D> mSpritesList;

    // draw data buffers
    std::vector<SpriteVertex3D> mDrawVertices;
    std::vector<DrawIndex> mDrawIndices;

    std::vector<DrawSpriteBatch> mBatchesList;
    TrimeshBuffer mTrimeshBuffer;

    DepthAxis mDepthAxis = DepthAxis_Y;
};