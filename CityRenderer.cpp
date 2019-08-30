#include "stdafx.h"
#include "CityRenderer.h"
#include "RenderSystem.h"
#include "GpuBuffer.h"
#include "CarnageGame.h"

bool CityRenderer::Initialize()
{
    mCityMeshBufferV = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
    debug_assert(mCityMeshBufferV);

    mCityMeshBufferI = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
    debug_assert(mCityMeshBufferI);

    if (mCityMeshBufferV == nullptr || mCityMeshBufferI == nullptr)
        return false;

    mCityMeshArea.SetNull();
    return true;
}

void CityRenderer::Deinit()
{
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
    mCityMeshArea.SetNull();
}

void CityRenderer::RenderFrame()
{
    BuildCityMeshData();

    RenderStates cityMeshRenderStates;

    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderSystem.mCityMeshProgram.Activate();
    gRenderSystem.mCityMeshProgram.UploadCameraTransformMatrices();
    gRenderSystem.mCityMeshProgram.SetTextureMappingEnabled(false);

    if (mCityMeshBufferV && mCityMeshBufferI)
    {
        gGraphicsDevice.BindVertexBuffer(mCityMeshBufferV, Vertex3D_Format::Get());
        gGraphicsDevice.BindIndexBuffer(mCityMeshBufferI);

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

void CityRenderer::UploadVertexData()
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
    int totalVertexDataBytes = totalVertexCount * Sizeof_Vertex3D;

    // upload vertex data
    mCityMeshBufferV->Setup(eBufferUsage_Static, totalVertexDataBytes, nullptr);
    if (void* pdata = mCityMeshBufferV->Lock(BufferAccess_Write))
    {
        char* pcursor = static_cast<char*>(pdata);
        for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
        {
            int dataLength = mCityMeshData[iLayer].mMeshVertices.size() * Sizeof_Vertex3D;
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

    int numBlocks = 32;

    int tilex = static_cast<int>(gCamera.mPosition.x / MAP_BLOCK_DIMS);
    int tiley = static_cast<int>(gCamera.mPosition.z / MAP_BLOCK_DIMS);

    for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
    {
        Rect2D rc(0, 0, 64, 64);
        mCityMeshBuilder.Build(gCarnageGame.mCityScape, rc, i, gRenderSystem.mCityRenderer.mCityMeshData[i]);
    }

    gRenderSystem.mCityRenderer.UploadVertexData();
}
