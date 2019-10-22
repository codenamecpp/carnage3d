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

bool MapRenderer::Initialize()
{
    mCityMeshBufferV = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
    debug_assert(mCityMeshBufferV);

    mCityMeshBufferI = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
    debug_assert(mCityMeshBufferI);

    if (mCityMeshBufferV == nullptr || mCityMeshBufferI == nullptr)
        return false;

    if (!mSpritesBatch.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize sprites batch");
        return false;
    }

    mCityMapRectangle.SetNull();
    return true;
}

void MapRenderer::Deinit()
{
    mSpritesBatch.Deinit();
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

void MapRenderer::RenderFrame()
{
    BuildMapMesh();

    gGraphicsDevice.BindTexture(eTextureUnit_3, gSpriteManager.mPalettesTable);
    gGraphicsDevice.BindTexture(eTextureUnit_2, gSpriteManager.mPaletteIndicesTable);

    DrawCityMesh();

    // collect and render game objects sprites
    for (Pedestrian* currPedestrian: gCarnageGame.mObjectsManager.mActivePedestriansList)
    {
        currPedestrian->DrawFrame(mSpritesBatch);
    }
    for (Vehicle* currVehicle: gCarnageGame.mObjectsManager.mActiveCarsList)
    {
        currVehicle->DrawFrame(mSpritesBatch);
    }
    mSpritesBatch.Flush();
}

void MapRenderer::CommitCityMeshData()
{
    int totalIndexCount = 0;
    int totalVertexCount = 0;

    for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
    {
        int numVertices = mCityMeshData[iLayer].mBlocksVertices.size();
        totalVertexCount += numVertices;

        int numIndices = mCityMeshData[iLayer].mBlocksIndices.size();
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
            int dataLength = mCityMeshData[iLayer].mBlocksVertices.size() * Sizeof_CityVertex3D;
            if (mCityMeshData[iLayer].mBlocksVertices.empty())
                continue;
            memcpy(pcursor, mCityMeshData[iLayer].mBlocksVertices.data(), dataLength);
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
            int dataLength = mCityMeshData[iLayer].mBlocksIndices.size() * Sizeof_DrawIndex_t;
            if (mCityMeshData[iLayer].mBlocksIndices.empty())
                continue;
            memcpy(pcursor, mCityMeshData[iLayer].mBlocksIndices.data(), dataLength);
            pcursor += dataLength;
        }
        mCityMeshBufferI->Unlock();
    }
}

void MapRenderer::BuildMapMesh()
{
    if (gGameCheatsWindow.mGenerateFullMeshForMap && mCityMapRectangle.h == 0 && mCityMapRectangle.w == 0)
    {
        mCityMapRectangle.x = 0;
        mCityMapRectangle.y = 0;
        mCityMapRectangle.w = MAP_DIMENSIONS;
        mCityMapRectangle.h = MAP_DIMENSIONS;  

        gConsole.LogMessage(eLogMessage_Debug, "City mesh invalidated [full]");
        for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
        {
            GameMapHelpers::BuildMapMesh(gGameMap, mCityMapRectangle, i, gRenderManager.mMapRenderer.mCityMeshData[i]);
        }
        gRenderManager.mMapRenderer.CommitCityMeshData();
        return;
    }

    int viewBlocks = 14;

    int tilex = static_cast<int>(gCamera.mPosition.x / MAP_BLOCK_LENGTH);
    int tiley = static_cast<int>(gCamera.mPosition.z / MAP_BLOCK_LENGTH);

    Rect2D rcMapView { -viewBlocks / 2 + tilex, -viewBlocks / 2 + tiley, viewBlocks, viewBlocks };

    bool invalidateCache = mCityMapRectangle.w == 0 || mCityMapRectangle.h == 0 || 
        rcMapView.x < mCityMapRectangle.x || rcMapView.y < mCityMapRectangle.y ||
        rcMapView.x + rcMapView.w > mCityMapRectangle.x + mCityMapRectangle.w ||
        rcMapView.y + rcMapView.h > mCityMapRectangle.y + mCityMapRectangle.h;

    if (!invalidateCache)
        return;

    gConsole.LogMessage(eLogMessage_Debug, "City mesh invalidated [partial]");

    int cacheNumBlocks = 32;
    mCityMapRectangle.x = (-cacheNumBlocks / 2) + tilex;
    mCityMapRectangle.y = (-cacheNumBlocks / 2) + tiley;
    mCityMapRectangle.w = cacheNumBlocks;
    mCityMapRectangle.h = cacheNumBlocks;

    for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
    {
        GameMapHelpers::BuildMapMesh(gGameMap, mCityMapRectangle, i, gRenderManager.mMapRenderer.mCityMeshData[i]);
    }
    gRenderManager.mMapRenderer.CommitCityMeshData();
}

void MapRenderer::DrawCityMesh()
{
    RenderStates cityMeshRenderStates;

    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderManager.mCityMeshProgram.Activate();
    gRenderManager.mCityMeshProgram.UploadCameraTransformMatrices();

    if (mCityMeshBufferV && mCityMeshBufferI)
    {
        gGraphicsDevice.BindVertexBuffer(mCityMeshBufferV, CityVertex3D_Format::Get());
        gGraphicsDevice.BindIndexBuffer(mCityMeshBufferI);
        gGraphicsDevice.BindTexture(eTextureUnit_0, gSpriteManager.mBlocksTextureArray);
        gGraphicsDevice.BindTexture(eTextureUnit_1, gSpriteManager.mBlocksIndicesTable);

        int currBaseVertex = 0;
        int currIndexOffset = 0;
        
        for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
        {
            int numIndices = mCityMeshData[i].mBlocksIndices.size();
            int numVertices = mCityMeshData[i].mBlocksVertices.size();
            if (gGameCheatsWindow.mDrawMapLayers[i])
            {
                int currIndexOffsetBytes = currIndexOffset * Sizeof_DrawIndex_t;
                gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, currIndexOffsetBytes, numIndices, currBaseVertex);
            }
            currIndexOffset += numIndices;
            currBaseVertex += numVertices;
        }
    }
    gRenderManager.mCityMeshProgram.Deactivate();
}

void MapRenderer::InvalidateMapMesh()
{
    mCityMapRectangle.SetNull();
}