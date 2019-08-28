#include "stdafx.h"
#include "CityRenderer.h"
#include "RenderSystem.h"
#include "GpuBuffer.h"

bool CityRenderer::Initialize()
{
    return true;
}

void CityRenderer::Deinit()
{
    if (mCityVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mCityVertexBuffer);
        mCityVertexBuffer = nullptr;
    }

    if (mCityIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mCityIndexBuffer);
        mCityIndexBuffer = nullptr;
    }

    for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
    {
        mCityLayersMeshData[iLayer].SetNull();
    }
}

void CityRenderer::RenderFrame()
{
    RenderStates cityMeshRenderStates;

    gGraphicsDevice.SetRenderStates(cityMeshRenderStates);

    gRenderSystem.mCityMeshProgram.Activate();
    gRenderSystem.mCityMeshProgram.UploadCameraTransformMatrices();

    if (mCityVertexBuffer && mCityIndexBuffer)
    {
        gGraphicsDevice.BindVertexBuffer(mCityVertexBuffer, Vertex3D_Format::Get());
        gGraphicsDevice.BindIndexBuffer(mCityIndexBuffer);

        int currBaseVertex = 0;
        int currIndexOffset = 0;
        
        for (int i = 0; i < MAP_LAYERS_COUNT; ++i)
        {
            int numIndices = mCityLayersMeshData[i].mMeshIndices.size();
            int numVertices = mCityLayersMeshData[i].mMeshVertices.size();
            int currIndexOffsetBytes = currIndexOffset * Sizeof_DrawIndex_t;

            gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, currIndexOffsetBytes, numIndices, currBaseVertex);

            currIndexOffset += numIndices;
            currBaseVertex += numVertices;
        }
    }

    gRenderSystem.mCityMeshProgram.Deactivate();
}

void CityRenderer::CommitVertexData()
{
    int totalIndexCount = 0;
    int totalVertexCount = 0;

    for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
    {
        int numVertices = mCityLayersMeshData[iLayer].mMeshVertices.size();
        totalVertexCount += numVertices;

        int numIndices = mCityLayersMeshData[iLayer].mMeshIndices.size();
        totalIndexCount += numIndices;
    }

    // allocate vertex buffer if it does not created yet
    if (mCityVertexBuffer == nullptr)
    {
        mCityVertexBuffer = gGraphicsDevice.CreateBuffer();
        debug_assert(mCityVertexBuffer);
    }

    // allocate index buffer if it does not created yet
    if (mCityIndexBuffer == nullptr)
    {
        mCityIndexBuffer = gGraphicsDevice.CreateBuffer();
        debug_assert(mCityIndexBuffer);
    }

    if (mCityVertexBuffer == nullptr || mCityIndexBuffer == nullptr || totalIndexCount == 0 || totalVertexCount == 0)
        return;

    int totalIndexDataBytes = totalIndexCount * Sizeof_DrawIndex_t;
    int totalVertexDataBytes = totalVertexCount * Sizeof_Vertex3D;

    // upload vertex data
    mCityVertexBuffer->Setup(eBufferContent_Vertices, eBufferUsage_Static, totalVertexDataBytes, nullptr);
    if (void* pdata = mCityVertexBuffer->Lock(BufferAccess_Write))
    {
        char* pcursor = static_cast<char*>(pdata);
        for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
        {
            int dataLength = mCityLayersMeshData[iLayer].mMeshVertices.size() * Sizeof_Vertex3D;
            if (mCityLayersMeshData[iLayer].mMeshVertices.empty())
                continue;
            memcpy(pcursor, mCityLayersMeshData[iLayer].mMeshVertices.data(), dataLength);
            pcursor += dataLength;
        }
        mCityVertexBuffer->Unlock();
    }

    // upload index data
    mCityIndexBuffer->Setup(eBufferContent_Indices, eBufferUsage_Static, totalIndexDataBytes, nullptr);
    if (void* pdata = mCityIndexBuffer->Lock(BufferAccess_Write))
    {
        char* pcursor = static_cast<char*>(pdata);
        for (int iLayer = 0; iLayer < MAP_LAYERS_COUNT; ++iLayer)
        {
            int dataLength = mCityLayersMeshData[iLayer].mMeshIndices.size() * Sizeof_DrawIndex_t;
            if (mCityLayersMeshData[iLayer].mMeshIndices.empty())
                continue;
            memcpy(pcursor, mCityLayersMeshData[iLayer].mMeshIndices.data(), dataLength);
            pcursor += dataLength;
        }
        mCityIndexBuffer->Unlock();
    }
}
