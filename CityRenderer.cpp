#include "stdafx.h"
#include "CityRenderer.h"
#include "RenderSystem.h"
#include "GpuBuffer.h"
#include "CarnageGame.h"
#include "SpriteCache.h"
#include "GpuTexture2D.h"

const unsigned int NumVerticesPerSprite = 4;
const unsigned int NumIndicesPerSprite = 6;

inline glm::vec2 rotate_around_center(glm::vec2 point, const glm::vec2& center, float angleRadians) 
{
    // substract center to use simplyfied rotation
    point -= center;
        
    float cos_a = std::cos(angleRadians);
    float sin_a = std::sin(angleRadians);
        
    float tmp_x = cos_a*point.x - sin_a*point.y;
    float tmp_y = sin_a*point.x + cos_a*point.y;
        
    point.x = tmp_x;
    point.y = tmp_y;
        
    // add center to move point to its original center position
    point += center;
    return point;
}

CityRenderer::CityRenderer()
{
    mDrawSpritesList.reserve(2048);
}

bool CityRenderer::Initialize()
{
    mCityMeshBufferV = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
    debug_assert(mCityMeshBufferV);

    mCityMeshBufferI = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
    debug_assert(mCityMeshBufferI);

    if (mCityMeshBufferV == nullptr || mCityMeshBufferI == nullptr)
        return false;

    if (!mSpritesVertexCache.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize sprites vertex cache");
        return false;
    }

    mCityMapRectangle.SetNull();
    return true;
}

void CityRenderer::Deinit()
{
    mSpritesVertexCache.Deinit();
    if (mCityMeshBufferV)
    {
        gGraphicsDevice.DestroyBuffer(mCityMeshBufferV);
        mCityMeshBufferV = nullptr;
    }

    if (mCityMeshBufferI)
    {
        gGraphicsDevice.DestroyBuffer(mCityMeshBufferI);
        mCityMeshBufferI = nullptr;
    }

    for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
    {
        mCityMeshData[iLayer].SetNull();
    }
    mCityMapRectangle.SetNull();
}

void CityRenderer::RenderFrame()
{
    BuildCityMeshData();

    RenderFrameBegin();

    DrawCityMesh();
    DrawPeds();
    DrawMapObjects();
    DrawCars();
    DrawProjectiles();

    if (!mDrawSpritesList.empty())
    {
        SortDrawSpritesList();
        SetDrawSpritesBatches();
        RenderDrawSpritesBatches();
    }

    RenderFrameEnd();
}

void CityRenderer::CommitCityMeshData()
{
    int totalIndexCount = 0;
    int totalVertexCount = 0;

    for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
    {
        int numVertices = mCityMeshData[iLayer].mMeshVertices.size();
        totalVertexCount += numVertices;

        int numIndices = mCityMeshData[iLayer].mMeshIndices.size();
        totalIndexCount += numIndices;
    }

    if (totalIndexCount == 0 || totalVertexCount == 0)
        return;

    int totalIndexDataBytes = totalIndexCount * Sizeof_DrawIndex_t;
    int totalVertexDataBytes = totalVertexCount * Sizeof_CityVertex3D;

    // upload vertex data
    mCityMeshBufferV->Setup(eBufferUsage_Static, totalVertexDataBytes, nullptr);
    if (void* pdata = mCityMeshBufferV->Lock(BufferAccess_Write))
    {
        char* pcursor = static_cast<char*>(pdata);
        for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
        {
            int dataLength = mCityMeshData[iLayer].mMeshVertices.size() * Sizeof_CityVertex3D;
            if (mCityMeshData[iLayer].mMeshVertices.empty())
                continue;
            memcpy(pcursor, mCityMeshData[iLayer].mMeshVertices.data(), dataLength);
            pcursor += dataLength;
        }
        mCityMeshBufferV->Unlock();
    }

    // upload index data
    mCityMeshBufferI->Setup(eBufferUsage_Static, totalIndexDataBytes, nullptr);
    if (void* pdata = mCityMeshBufferI->Lock(BufferAccess_Write))
    {
        char* pcursor = static_cast<char*>(pdata);
        for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
        {
            int dataLength = mCityMeshData[iLayer].mMeshIndices.size() * Sizeof_DrawIndex_t;
            if (mCityMeshData[iLayer].mMeshIndices.empty())
                continue;
            memcpy(pcursor, mCityMeshData[iLayer].mMeshIndices.data(), dataLength);
            pcursor += dataLength;
        }
        mCityMeshBufferI->Unlock();
    }
}

void CityRenderer::BuildCityMeshData()
{
    static bool isBuilt = false; // temporary!
    if (isBuilt)
        return;

    isBuilt = true;

    int numBlocks = 30;

    int tilex = static_cast<int>(gCamera.mPosition.x / MAP_BLOCK_DIMS);
    int tiley = static_cast<int>(gCamera.mPosition.z / MAP_BLOCK_DIMS);

    for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
    {
        Rect2D rc(0, 0, numBlocks, numBlocks);
        mCityMeshBuilder.Build(gCarnageGame.mCityScape, rc, i, gRenderSystem.mCityRenderer.mCityMeshData[i]);
    }

    gRenderSystem.mCityRenderer.CommitCityMeshData();
}

void CityRenderer::RenderFrameBegin()
{
}

void CityRenderer::RenderFrameEnd()
{
    mSpritesVertexCache.FlushCache();
    mDrawSpritesList.clear();
    mDrawSpritesBatchesList.clear();
    mDrawSpritesVertices.clear();
    mDrawSpritesIndices.clear();
}

void CityRenderer::DrawCityMesh()
{
    RenderStates cityMeshRenderStates;

    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderSystem.mCityMeshProgram.Activate();
    gRenderSystem.mCityMeshProgram.UploadCameraTransformMatrices();

    GpuTextureArray2D* blocksTextureArray = gSpriteCache.mBlocksTextureArray2D;
    gRenderSystem.mCityMeshProgram.SetTextureMappingEnabled(blocksTextureArray != nullptr);

    if (mCityMeshBufferV && mCityMeshBufferI)
    {
        gGraphicsDevice.BindVertexBuffer(mCityMeshBufferV, CityVertex3D_Format::Get());
        gGraphicsDevice.BindIndexBuffer(mCityMeshBufferI);
        gGraphicsDevice.BindTextureArray2D(eTextureUnit_0, blocksTextureArray);

        int currBaseVertex = 0;
        int currIndexOffset = 0;
        
        for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
        {
            int numIndices = mCityMeshData[i].mMeshIndices.size();
            int numVertices = mCityMeshData[i].mMeshVertices.size();

            int currIndexOffsetBytes = currIndexOffset * Sizeof_DrawIndex_t;
            gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, currIndexOffsetBytes, numIndices, currBaseVertex);

            currIndexOffset += numIndices;
            currBaseVertex += numVertices;
        }
    }
    gRenderSystem.mCityMeshProgram.Deactivate();
}

void CityRenderer::DrawSprite3D(GpuTexture2D* texture, const Rect2D& rc, const glm::vec3& position, bool centerOrigin, float sprScale, float heading)
{
    float tinvx = 1.0f / texture->mSize.x;
    float tinvy = 1.0f / texture->mSize.y;

    // setup draw sprite record
    DrawSpriteRec rec;
        rec.mPosition.x = position.x;
        rec.mPosition.y = position.y;
        rec.mSize.x = rc.w * sprScale;
        rec.mSize.y = rc.h * sprScale;
        rec.mCenterOffset.x = centerOrigin ? (-rec.mSize.x * 0.5f) : 0.0f;
        rec.mCenterOffset.y = centerOrigin ? (-rec.mSize.y * 0.5f) : 0.0f;
        rec.mTcUv0.x = rc.x * tinvx;
        rec.mTcUv0.y = rc.y * tinvy;
        rec.mTcUv1.x = (rc.x + rc.w) * tinvx;
        rec.mTcUv1.y = (rc.y + rc.h) * tinvy;
        rec.mHeading = heading;
        rec.mDepth = position.z;
        rec.mSpriteTexture = texture;
    mDrawSpritesList.push_back(rec);
}

void CityRenderer::DrawSprite2D(GpuTexture2D* texture, const Rect2D& rc, const glm::vec2& position, bool centerOrigin, float sprScale, float heading)
{
    float tinvx = 1.0f / texture->mSize.x;
    float tinvy = 1.0f / texture->mSize.y;

    // setup draw sprite record
    DrawSpriteRec rec;
        rec.mPosition.x = position.x;
        rec.mPosition.y = position.y;
        rec.mSize.x = rc.w * sprScale;
        rec.mSize.y = rc.h * sprScale;
        rec.mCenterOffset.x = centerOrigin ? (-rec.mSize.x * 0.5f) : 0.0f;
        rec.mCenterOffset.y = centerOrigin ? (-rec.mSize.y * 0.5f) : 0.0f;
        rec.mTcUv0.x = rc.x * tinvx;
        rec.mTcUv0.y = rc.y * tinvy;
        rec.mTcUv1.x = (rc.x + rc.w) * tinvx;
        rec.mTcUv1.y = (rc.y + rc.h) * tinvy;
        rec.mHeading = heading;
        rec.mDepth = 0.0f;
        rec.mSpriteTexture = texture;
    mDrawSpritesList.push_back(rec);
}

void CityRenderer::DrawPeds()
{
    CityStyleData& style = gCarnageGame.mCityScape.mStyleData; // todo

    float spriteScale = (1.0f / MAP_PIXELS_PER_TILE);
    for (Pedestrian* currPedestrian: gCarnageGame.mPedsManager.mActivePedsList)
    {
        if (currPedestrian == nullptr)
            continue;

        int spriteLinearIndex = style.GetSpriteIndex(eSpriteType_Ped, currPedestrian->mSprite);
        DrawSprite3D(gSpriteCache.mObjectsSpritesheet.mSpritesheetTexture, 
            gSpriteCache.mObjectsSpritesheet.mEtries[spriteLinearIndex].mRectangle, currPedestrian->mPosition, true, spriteScale, currPedestrian->mRotation);
    }
}

void CityRenderer::DrawCars()
{
}

void CityRenderer::DrawMapObjects()
{
}

void CityRenderer::DrawProjectiles()
{
}

void CityRenderer::SortDrawSpritesList()
{
    std::sort(mDrawSpritesList.begin(), mDrawSpritesList.end(), [](const DrawSpriteRec& lhs, const DrawSpriteRec& rhs)
        {
            return lhs.mSpriteTexture < rhs.mSpriteTexture;
        });
}

void CityRenderer::SetDrawSpritesBatches()
{
    int numSprites = mDrawSpritesList.size();

    int totalVertexCount = numSprites * NumVerticesPerSprite; 
    debug_assert(totalVertexCount > 0);

    int totalIndexCount = numSprites * NumIndicesPerSprite; 
    debug_assert(totalIndexCount > 0);

    // allocate memory for mesh data
    mDrawSpritesVertices.resize(totalVertexCount);
    SpriteVertex3D* vertexData = mDrawSpritesVertices.data();

    mDrawSpritesIndices.resize(totalIndexCount);
    DrawIndex_t* indexData = mDrawSpritesIndices.data();

    // initial batch
    mDrawSpritesBatchesList.clear();
    mDrawSpritesBatchesList.emplace_back();
    DrawSpriteBatch* currentBatch = &mDrawSpritesBatchesList.back();
    currentBatch->mFirstVertex = 0;
    currentBatch->mFirstIndex = 0;
    currentBatch->mVertexCount = 0;
    currentBatch->mIndexCount = 0;
    currentBatch->mSpriteTexture = mDrawSpritesList[0].mSpriteTexture;

    for (int isprite = 0; isprite < numSprites; ++isprite)
    {
        const DrawSpriteRec& sprite = mDrawSpritesList[isprite];
        // start new batch
        if (sprite.mSpriteTexture != currentBatch->mSpriteTexture)
        {
            DrawSpriteBatch newBatch;
            newBatch.mFirstVertex = currentBatch->mVertexCount;
            newBatch.mFirstIndex = currentBatch->mIndexCount;
            newBatch.mVertexCount = 0;
            newBatch.mIndexCount = 0;
            newBatch.mSpriteTexture = sprite.mSpriteTexture;
            mDrawSpritesBatchesList.push_back(newBatch);
            currentBatch = &mDrawSpritesBatchesList.back();
        }

        currentBatch->mVertexCount += NumVerticesPerSprite;   
        currentBatch->mIndexCount += NumIndicesPerSprite;

        int vertexOffset = isprite * NumVerticesPerSprite;

        vertexData[vertexOffset + 0].mTexcoord.x = sprite.mTcUv0.x;
        vertexData[vertexOffset + 0].mTexcoord.y = sprite.mTcUv0.y;
        vertexData[vertexOffset + 0].mPosition.y = sprite.mDepth;

        vertexData[vertexOffset + 1].mTexcoord.x = sprite.mTcUv1.x;
        vertexData[vertexOffset + 1].mTexcoord.y = sprite.mTcUv0.y;
        vertexData[vertexOffset + 1].mPosition.y = sprite.mDepth;

        vertexData[vertexOffset + 2].mTexcoord.x = sprite.mTcUv0.x;
        vertexData[vertexOffset + 2].mTexcoord.y = sprite.mTcUv1.y;
        vertexData[vertexOffset + 2].mPosition.y = sprite.mDepth;

        vertexData[vertexOffset + 3].mTexcoord.x = sprite.mTcUv1.x;
        vertexData[vertexOffset + 3].mTexcoord.y = sprite.mTcUv1.y;
        vertexData[vertexOffset + 3].mPosition.y = sprite.mDepth;

        const glm::vec2 positions[4] = 
        {
            {sprite.mPosition.x + sprite.mCenterOffset.x,                   sprite.mPosition.y + sprite.mCenterOffset.y},
            {sprite.mPosition.x + sprite.mSize.x + sprite.mCenterOffset.x,  sprite.mPosition.y + sprite.mCenterOffset.y},
            {sprite.mPosition.x + sprite.mCenterOffset.x,                   sprite.mPosition.y + sprite.mSize.y + sprite.mCenterOffset.y},
            {sprite.mPosition.x + sprite.mSize.x + sprite.mCenterOffset.x,  sprite.mPosition.y + sprite.mSize.y + sprite.mCenterOffset.y},
        };

        if (fabs(sprite.mHeading) > 0.01f) // has rotation
        {
            for (int i = 0; i < 4; ++i)
            {
                glm::vec2 currPos = rotate_around_center(positions[i], sprite.mPosition, sprite.mHeading);

                vertexData[vertexOffset + i].mPosition.x = currPos.x;
                vertexData[vertexOffset + i].mPosition.z = currPos.y;
            }
        }
        else // no rotation
        {
            for (int i = 0; i < 4; ++i)
            {
                vertexData[vertexOffset + i].mPosition.x = positions[i].x;
                vertexData[vertexOffset + i].mPosition.z = positions[i].y;
            }
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

void CityRenderer::RenderDrawSpritesBatches()
{
    RenderStates cityMeshRenderStates;
    cityMeshRenderStates.Disable(RenderStateFlags_FaceCulling);
    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderSystem.mSpritesProgram.Activate();
    gRenderSystem.mSpritesProgram.UploadCameraTransformMatrices();

    GpuTextureArray2D* blocksTextureArray = gSpriteCache.mBlocksTextureArray2D;
    gRenderSystem.mSpritesProgram.SetTextureMappingEnabled(blocksTextureArray != nullptr);

    TransientBuffer vBuffer;
    TransientBuffer iBuffer;
    if (!mSpritesVertexCache.AllocVertex(Sizeof_SpriteVertex3D * mDrawSpritesVertices.size(), mDrawSpritesVertices.data(), vBuffer))
    {
        debug_assert(false);
        return;
    }

    if (!mSpritesVertexCache.AllocIndex(Sizeof_DrawIndex_t * mDrawSpritesIndices.size(), mDrawSpritesIndices.data(), iBuffer))
    {
        debug_assert(false);
        return;
    }

    SpriteVertex3D_Format vFormat;
    vFormat.mBaseOffset = vBuffer.mBufferDataOffset;

    gGraphicsDevice.BindVertexBuffer(vBuffer.mGraphicsBuffer, vFormat);
    gGraphicsDevice.BindIndexBuffer(iBuffer.mGraphicsBuffer);

    for (const DrawSpriteBatch& currBatch: mDrawSpritesBatchesList)
    {
        gGraphicsDevice.BindTexture2D(eTextureUnit_0, currBatch.mSpriteTexture);

        unsigned int idxBufferOffset = iBuffer.mBufferDataOffset + Sizeof_DrawIndex_t * currBatch.mFirstIndex;
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, idxBufferOffset, currBatch.mIndexCount);
    }

    gRenderSystem.mSpritesProgram.Deactivate();
}
