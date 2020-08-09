#include "stdafx.h"
#include "MapRenderer.h"
#include "RenderingManager.h"
#include "GpuBuffer.h"
#include "CarnageGame.h"
#include "SpriteManager.h"
#include "GpuTexture2D.h"
#include "GameCheatsWindow.h"
#include "PhysicsComponents.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "RenderView.h"

//////////////////////////////////////////////////////////////////////////

void MapRenderStats::FrameBegin()
{
    mBlockChunksDrawnCount = 0;
}

void MapRenderStats::FrameEnd()
{
}

//////////////////////////////////////////////////////////////////////////

bool MapRenderer::Initialize()
{
    mCityMeshBufferV = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
    debug_assert(mCityMeshBufferV);

    mCityMeshBufferI = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
    debug_assert(mCityMeshBufferI);

    if (mCityMeshBufferV == nullptr || mCityMeshBufferI == nullptr)
        return false;

    if (!mSpriteBatch.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize sprites batch");
        return false;
    }

    return true;
}

void MapRenderer::Deinit()
{
    mSpriteBatch.Deinit();
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
}

void MapRenderer::RenderFrameBegin()
{
    mRenderStats.FrameBegin();
}

void MapRenderer::RenderFrameEnd()
{
    mRenderStats.FrameEnd();
}

void MapRenderer::RenderFrame(RenderView* renderview)
{
    debug_assert(renderview);

    gGraphicsDevice.BindTexture(eTextureUnit_3, gSpriteManager.mPalettesTable);
    gGraphicsDevice.BindTexture(eTextureUnit_2, gSpriteManager.mPaletteIndicesTable);

    DrawCityMesh(renderview);

    mSpriteBatch.BeginBatch(SpriteBatch::DepthAxis_Y);

    // collect and render game objects sprites - the order matters

    for (Vehicle* currGameObject: gGameObjectsManager.mCarsList)
    {
        currGameObject->DrawFrame(mSpriteBatch);
    }

    for (Pedestrian* currGameObject: gGameObjectsManager.mPedestriansList)
    {
        currGameObject->DrawFrame(mSpriteBatch);
    }

    gRenderManager.mSpritesProgram.Activate();
    gRenderManager.mSpritesProgram.UploadCameraTransformMatrices(renderview->mCamera);

    RenderStates guiRenderStates = RenderStates().Disable(RenderStateFlags_FaceCulling);
    gGraphicsDevice.SetRenderStates(guiRenderStates);

    mSpriteBatch.Flush();

    gRenderManager.mSpritesProgram.Deactivate();
}

void MapRenderer::RenderDebug(RenderView* renderview, DebugRenderer& debugRender)
{
    debug_assert(renderview);
    for (GameObject* currGameObject: gGameObjectsManager.mObjectsList)
    {
        currGameObject->DrawDebug(debugRender);
    }
}

void MapRenderer::DrawCityMesh(RenderView* renderview)
{
    RenderStates cityMeshRenderStates;

    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderManager.mCityMeshProgram.Activate();
    gRenderManager.mCityMeshProgram.UploadCameraTransformMatrices(renderview->mCamera);

    if (mCityMeshBufferV && mCityMeshBufferI)
    {
        gGraphicsDevice.BindVertexBuffer(mCityMeshBufferV, CityVertex3D_Format::Get());
        gGraphicsDevice.BindIndexBuffer(mCityMeshBufferI);
        gGraphicsDevice.BindTexture(eTextureUnit_0, gSpriteManager.mBlocksTextureArray);
        gGraphicsDevice.BindTexture(eTextureUnit_1, gSpriteManager.mBlocksIndicesTable);

        for (const MapBlocksChunk& currChunk: mMapBlocksChunks)
        {
            if (!renderview->mCamera.mFrustum.contains(currChunk.mBounds))
                continue;

            gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, 
                currChunk.mIndicesStart * Sizeof_DrawIndex, currChunk.mIndicesCount);

            ++mRenderStats.mBlockChunksDrawnCount;
        }
    }
    gRenderManager.mCityMeshProgram.Deactivate();
}

void MapRenderer::BuildMapMesh()
{
    MapMeshData blocksMesh;
    for (int batchy = 0; batchy < BlocksBatchesPerSide; ++batchy)
    {
        for (int batchx = 0; batchx < BlocksBatchesPerSide; ++batchx)
        {
            Rectangle mapArea { 
                batchx * BlocksBatchDims - ExtraBlocksPerSide, 
                batchy * BlocksBatchDims - ExtraBlocksPerSide,
                BlocksBatchDims,
                BlocksBatchDims };

            unsigned int prevVerticesCount = blocksMesh.mBlocksVertices.size();
            unsigned int prevIndicesCount = blocksMesh.mBlocksIndices.size();

            MapBlocksChunk& currChunk = mMapBlocksChunks[batchy * BlocksBatchesPerSide + batchx];
            currChunk.mBounds.mMin = glm::vec3 { mapArea.x * MAP_BLOCK_LENGTH, 0, mapArea.y * MAP_BLOCK_LENGTH };
            currChunk.mBounds.mMax = glm::vec3 { 
                (mapArea.x + mapArea.w) * MAP_BLOCK_LENGTH, MAP_LAYERS_COUNT * MAP_BLOCK_LENGTH, 
                (mapArea.y + mapArea.h) * MAP_BLOCK_LENGTH };

            currChunk.mVerticesStart = prevVerticesCount;
            currChunk.mIndicesStart = prevIndicesCount;
            
            // append new geometry
            GameMapHelpers::BuildMapMesh(gGameMap, mapArea, blocksMesh);
            
            currChunk.mVerticesCount = blocksMesh.mBlocksVertices.size() - prevVerticesCount;
            currChunk.mIndicesCount = blocksMesh.mBlocksIndices.size() - prevIndicesCount;
        }
    }

    // upload map geometry to video memory
    int totalVertexDataBytes = blocksMesh.mBlocksVertices.size() * Sizeof_CityVertex3D;
    int totalIndexDataBytes = blocksMesh.mBlocksIndices.size() * Sizeof_DrawIndex;

    // upload vertex data
    mCityMeshBufferV->Setup(eBufferUsage_Static, totalVertexDataBytes, nullptr);
    if (void* pdata = mCityMeshBufferV->Lock(BufferAccess_Write))
    {
        memcpy(pdata, blocksMesh.mBlocksVertices.data(), totalVertexDataBytes);
        mCityMeshBufferV->Unlock();
    }

    // upload index data
    mCityMeshBufferI->Setup(eBufferUsage_Static, totalIndexDataBytes, nullptr);
    if (void* pdata = mCityMeshBufferI->Lock(BufferAccess_Write))
    {
        memcpy(pdata, blocksMesh.mBlocksIndices.data(), totalIndexDataBytes);
        mCityMeshBufferI->Unlock();
    }
}