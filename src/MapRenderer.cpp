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
    mSpritesDrawnCount = 0;

    ++mRenderFramesCounter;
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

    if (gGameCheatsWindow.mEnableDrawCityMesh)
    {
        DrawCityMesh(renderview);
    }

    mSpriteBatch.BeginBatch(SpriteBatch::DepthAxis_Y);

    // collect and render game objects sprites
    for (GameObject* currObject: gGameObjectsManager.mAllObjectsList)
    {
        // attached objects must be drawn after the object to which they are attached
        if (currObject->IsAttachedToObject())
            continue;

        DrawGameObject(renderview, currObject);
    }

    gRenderManager.mSpritesProgram.Activate();
    gRenderManager.mSpritesProgram.UploadCameraTransformMatrices(renderview->mCamera);

    RenderStates guiRenderStates = RenderStates().Disable(RenderStateFlags_FaceCulling);
    gGraphicsDevice.SetRenderStates(guiRenderStates);

    mSpriteBatch.Flush();

    gRenderManager.mSpritesProgram.Deactivate();
}

void MapRenderer::DrawGameObject(RenderView* renderview, GameObject* gameObject)
{
    if (gameObject->IsMarkedForDeletion() || gameObject->IsInvisibleFlag())
        return;

    bool dbgSkipDraw = 
        (!gGameCheatsWindow.mEnableDrawPedestrians && gameObject->IsPedestrianClass()) ||
        (!gGameCheatsWindow.mEnableDrawVehicles && gameObject->IsVehicleClass()) ||
        (!gGameCheatsWindow.mEnableDrawObstacles && gameObject->IsObstacleClass()) ||
        (!gGameCheatsWindow.mEnableDrawDecorations && gameObject->IsDecorationClass());

    if (!dbgSkipDraw)
    {
        gameObject->PreDrawFrame();

        // detect if gameobject is visible on screen
        const glm::vec2 spritePosition = (gameObject->mDrawSprite.mPosition + gameObject->mDrawSprite.GetOriginPoint());
        float maxdistance = Convert::MapUnitsToMeters(10.0f); // todo: magic numbers
        if (fabs(renderview->mCamera.mPosition.x - spritePosition.x) > maxdistance ||
            fabs(renderview->mCamera.mPosition.z - spritePosition.y) > maxdistance)
        {
            // skip
        }
        else
        {
            mSpriteBatch.DrawSprite(gameObject->mDrawSprite);

            ++mRenderStats.mSpritesDrawnCount;
            gameObject->mLastRenderFrame = mRenderStats.mRenderFramesCounter;
        }
    }

    if (!gameObject->HasAttachedObjects())
        return;

    // draw attached objects
    for (int ichild = 0; ; ++ichild)
    {
        GameObject* currentChild = gameObject->GetAttachedObject(ichild);
        if (currentChild == nullptr)
            break;

        DrawGameObject(renderview, currentChild);
    }
}

void MapRenderer::RenderDebug(RenderView* renderview, DebugRenderer& debugRender)
{
    debug_assert(renderview);
    for (GameObject* gameObject: gGameObjectsManager.mAllObjectsList)
    {
        // check if gameobject was on screen in current frame
        if (gameObject->mLastRenderFrame != mRenderStats.mRenderFramesCounter)
            continue;

        gameObject->DrawDebug(debugRender);
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
    CityMeshData blocksMesh;
    for (int batchy = 0; batchy < BlocksBatchesPerSide; ++batchy)
    {
        for (int batchx = 0; batchx < BlocksBatchesPerSide; ++batchx)
        {
            Rect mapArea { 
                batchx * BlocksBatchDims - ExtraBlocksPerSide, 
                batchy * BlocksBatchDims - ExtraBlocksPerSide,
                BlocksBatchDims,
                BlocksBatchDims };

            unsigned int prevVerticesCount = blocksMesh.mBlocksVertices.size();
            unsigned int prevIndicesCount = blocksMesh.mBlocksIndices.size();

            MapBlocksChunk& currChunk = mMapBlocksChunks[batchy * BlocksBatchesPerSide + batchx];
            currChunk.mBounds.mMin = glm::vec3 { mapArea.x * METERS_PER_MAP_UNIT, 0.0f, mapArea.y * METERS_PER_MAP_UNIT };
            currChunk.mBounds.mMax = glm::vec3 { 
                (mapArea.x + mapArea.w) * METERS_PER_MAP_UNIT, MAP_LAYERS_COUNT * METERS_PER_MAP_UNIT, 
                (mapArea.y + mapArea.h) * METERS_PER_MAP_UNIT};

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