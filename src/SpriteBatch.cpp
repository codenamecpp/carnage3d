#include "stdafx.h"
#include "SpriteBatch.h"
#include "RenderingManager.h"
#include "SpriteManager.h"

const unsigned int NumVerticesPerSprite = 4;
const unsigned int NumIndicesPerSprite = 6;

bool SpriteBatch::Initialize()
{
    mSpritesList.reserve(1024);
    return true;
}

void SpriteBatch::Deinit()
{
    Clear();
}

void SpriteBatch::Clear()
{
    mSpritesList.clear();
    mDrawVertices.clear();
    mDrawIndices.clear();
    mBatchesList.clear();
}

void SpriteBatch::SortSpritesList()
{
    //std::sort(mSpritesList.begin(), mSpritesList.end(), [](const Sprite& lhs, const Sprite& rhs)
    //    {
    //        return lhs.mTexture < rhs.mTexture;
    //    });
}

void SpriteBatch::DrawSprite(const Sprite2D& sourceSprite)
{
    mSpritesList.push_back(sourceSprite);
}

void SpriteBatch::Flush()
{
    if (!mSpritesList.empty())
    {
        SortSpritesList();
        GenerateSpritesBatches();
        RenderSpritesBatches();
    }
    Clear();
}

void SpriteBatch::GenerateSpritesBatches()
{
    int numSprites = mSpritesList.size();

    int totalVertexCount = numSprites * NumVerticesPerSprite; 
    debug_assert(totalVertexCount > 0);

    int totalIndexCount = numSprites * NumIndicesPerSprite; 
    debug_assert(totalIndexCount > 0);

    // allocate memory for mesh data
    mDrawVertices.resize(totalVertexCount);
    SpriteVertex3D* vertexData = mDrawVertices.data();

    mDrawIndices.resize(totalIndexCount);
    DrawIndex_t* indexData = mDrawIndices.data();

    // initial batch
    mBatchesList.clear();
    mBatchesList.emplace_back();
    DrawSpriteBatch* currentBatch = &mBatchesList.back();
    currentBatch->mFirstVertex = 0;
    currentBatch->mFirstIndex = 0;
    currentBatch->mVertexCount = 0;
    currentBatch->mIndexCount = 0;
    currentBatch->mSpriteTexture = mSpritesList[0].mTexture;

    for (int isprite = 0; isprite < numSprites; ++isprite)
    {
        const Sprite2D& sprite = mSpritesList[isprite];
        // start new batch
        if (sprite.mTexture != currentBatch->mSpriteTexture)
        {
            DrawSpriteBatch newBatch;
            newBatch.mFirstVertex = currentBatch->mVertexCount + currentBatch->mFirstVertex;
            newBatch.mFirstIndex = currentBatch->mIndexCount + currentBatch->mFirstIndex;
            newBatch.mVertexCount = 0;
            newBatch.mIndexCount = 0;
            newBatch.mSpriteTexture = sprite.mTexture;
            mBatchesList.push_back(newBatch);
            currentBatch = &mBatchesList.back();
        }

        currentBatch->mVertexCount += NumVerticesPerSprite;   
        currentBatch->mIndexCount += NumIndicesPerSprite;

        int vertexOffset = isprite * NumVerticesPerSprite;

        vertexData[vertexOffset + 0].mTexcoord.x = sprite.mTextureRegion.mU0;
        vertexData[vertexOffset + 0].mTexcoord.y = sprite.mTextureRegion.mV0;
        vertexData[vertexOffset + 0].mPosition.y = sprite.mHeight;
        vertexData[vertexOffset + 0].mClutIndex = sprite.mClutIndex;

        vertexData[vertexOffset + 1].mTexcoord.x = sprite.mTextureRegion.mU1;
        vertexData[vertexOffset + 1].mTexcoord.y = sprite.mTextureRegion.mV0;
        vertexData[vertexOffset + 1].mPosition.y = sprite.mHeight;
        vertexData[vertexOffset + 1].mClutIndex = sprite.mClutIndex;

        vertexData[vertexOffset + 2].mTexcoord.x = sprite.mTextureRegion.mU0;
        vertexData[vertexOffset + 2].mTexcoord.y = sprite.mTextureRegion.mV1;
        vertexData[vertexOffset + 2].mPosition.y = sprite.mHeight;
        vertexData[vertexOffset + 2].mClutIndex = sprite.mClutIndex;

        vertexData[vertexOffset + 3].mTexcoord.x = sprite.mTextureRegion.mU1;
        vertexData[vertexOffset + 3].mTexcoord.y = sprite.mTextureRegion.mV1;
        vertexData[vertexOffset + 3].mPosition.y = sprite.mHeight;
        vertexData[vertexOffset + 3].mClutIndex = sprite.mClutIndex;

        glm::vec2 positions[4];
        sprite.GetCorners(positions);

        for (int i = 0; i < 4; ++i)
        {
            vertexData[vertexOffset + i].mPosition.x = positions[i].x;
            vertexData[vertexOffset + i].mPosition.z = positions[i].y;
        }

        // setup indices
        int indexOffset = isprite * NumIndicesPerSprite;
        indexData[indexOffset + 0] = vertexOffset + 0;
        indexData[indexOffset + 1] = vertexOffset + 1;
        indexData[indexOffset + 2] = vertexOffset + 2;
        indexData[indexOffset + 3] = vertexOffset + 1;
        indexData[indexOffset + 4] = vertexOffset + 2;
        indexData[indexOffset + 5] = vertexOffset + 3;
    }
}

void SpriteBatch::RenderSpritesBatches()
{
    FrameVertexCache& vertscache = gRenderManager.mFrameVertexCache;

    RenderStates cityMeshRenderStates;
    cityMeshRenderStates.Disable(RenderStateFlags_FaceCulling);
    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderManager.mSpritesProgram.Activate();
    gRenderManager.mSpritesProgram.UploadCameraTransformMatrices();

    TransientBuffer vBuffer;
    TransientBuffer iBuffer;
    if (!vertscache.AllocVertex(Sizeof_SpriteVertex3D * mDrawVertices.size(), mDrawVertices.data(), vBuffer))
    {
        debug_assert(false);
        return;
    }

    if (!vertscache.AllocIndex(Sizeof_DrawIndex_t * mDrawIndices.size(), mDrawIndices.data(), iBuffer))
    {
        debug_assert(false);
        return;
    }

    SpriteVertex3D_Format vFormat;
    vFormat.mBaseOffset = vBuffer.mBufferDataOffset;

    gGraphicsDevice.BindVertexBuffer(vBuffer.mGraphicsBuffer, vFormat);
    gGraphicsDevice.BindIndexBuffer(iBuffer.mGraphicsBuffer);

    for (const DrawSpriteBatch& currBatch: mBatchesList)
    {
        gGraphicsDevice.BindTexture(eTextureUnit_0, currBatch.mSpriteTexture);

        unsigned int idxBufferOffset = iBuffer.mBufferDataOffset + Sizeof_DrawIndex_t * currBatch.mFirstIndex;
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, idxBufferOffset, currBatch.mIndexCount);
    }

    gRenderManager.mSpritesProgram.Deactivate();
}