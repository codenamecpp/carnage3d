#include "stdafx.h"
#include "CityMeshBuilder.h"
#include "SpriteCache.h"

bool CityMeshBuilder::Build(CityScapeData& cityScape, const Rect2D& area, int layerIndex, CityBlocksMeshData& meshData)
{
    debug_assert(layerIndex > -1 && layerIndex < MAP_LAYERS_COUNT);
    debug_assert((area.x > -1 && area.y > -1) && 
        (area.w > 0 && area.h > 0) &&
        (area.x + area.w <= MAP_DIMENSIONS) && 
        (area.y + area.h <= MAP_DIMENSIONS));

    meshData.SetNull();

    // preallocate
    meshData.mMeshIndices.reserve(1 * 1024 * 1024);
    meshData.mMeshVertices.reserve(1 * 1024 * 1024);

    // prepare
    for (int tiley = 0; tiley < area.h; ++tiley)
    for (int tilex = 0; tilex < area.w; ++tilex)
    {
        if (BlockStyleData* blockInfo = cityScape.GetBlock(tilex + area.x, tiley + area.y, layerIndex))
        {
            for (int iface = 0; iface < eBlockFace_COUNT; ++iface)
            {
                if (blockInfo->mFaces[iface] == 0)
                    continue;

                eBlockFace faceid = (eBlockFace) iface;
                PutBlockFace(cityScape, meshData, tilex + area.x, tiley + area.y, layerIndex, faceid, blockInfo);
            }
        }
    }
    return true;
}

bool CityMeshBuilder::Build(CityScapeData& cityScape, const Rect2D& area, CityBlocksMeshData& meshData)
{
    debug_assert((area.x > -1 && area.y > -1) && 
        (area.w > 0 && area.h > 0) &&
        (area.x + area.w < MAP_DIMENSIONS) && 
        (area.y + area.h < MAP_DIMENSIONS));

    meshData.SetNull();

    // preallocate
    meshData.mMeshIndices.reserve(1 * 1024 * 1024);
    meshData.mMeshVertices.reserve(1 * 1024 * 1024);

    // prepare
    for (int tilez = 0; tilez < MAP_LAYERS_COUNT; ++tilez)
    for (int tiley = 0; tiley < area.h; ++tiley)
    for (int tilex = 0; tilex < area.w; ++tilex)
    {
        if (BlockStyleData* blockInfo = cityScape.GetBlock(tilex + area.x, tiley + area.y, tilez))
        {
            for (int iface = 0; iface < eBlockFace_COUNT; ++iface)
            {
                if (blockInfo->mFaces[iface] == 0)
                    continue;

                eBlockFace faceid = (eBlockFace) iface;
                PutBlockFace(cityScape, meshData, tilex + area.x, tiley + area.y, tilez, faceid, blockInfo);
            }
        }
    }
    return true;
}

void CityMeshBuilder::PutBlockFace(CityScapeData& cityScape, CityBlocksMeshData& meshData, int posx, int posy, int posz, eBlockFace face, BlockStyleData* blockInfo)
{
    assert(blockInfo && blockInfo->mFaces[face]);
    eBlockType blockType = (face == eBlockFace_Lid) ? eBlockType_Lid : eBlockType_Side;

    const int blockTexIndex = cityScape.mStyleData.GetBlockTextureLinearIndex(blockType, blockInfo->mFaces[face]);

    // setup base cube vertices
    glm::vec3 cubePoints[] =
    {
        // front face, cw
        { 0.0f,             0.0f,               MAP_BLOCK_DIMS }, 
        { MAP_BLOCK_DIMS,   0.0f,               MAP_BLOCK_DIMS }, 
        { MAP_BLOCK_DIMS,   -MAP_BLOCK_DIMS,    MAP_BLOCK_DIMS }, 
        { 0.0f,             -MAP_BLOCK_DIMS,    MAP_BLOCK_DIMS },
        // back face, cw
        { 0.0f,             0.0f,               0.0f }, 
        { MAP_BLOCK_DIMS,   0.0f,               0.0f }, 
        { MAP_BLOCK_DIMS,   -MAP_BLOCK_DIMS,    0.0f }, 
        { 0.0f,             -MAP_BLOCK_DIMS,    0.0f },
    };

    // process slope
    const int slope = blockInfo->mSlopeType;
    switch (slope)
    {
        // N, 26 low, high
        case 1: case 2:
            cubePoints[0].y = cubePoints[1].y = ((slope - 1) / 2.0f) - MAP_BLOCK_DIMS;
            cubePoints[4].y = cubePoints[5].y = ((slope - 1 + 1) / 2.0f) - MAP_BLOCK_DIMS;
        break;
        // S, 26 low, high
        case 3: case 4:
            cubePoints[4].y = cubePoints[5].y = ((slope - 3) / 2.0f) - MAP_BLOCK_DIMS;
            cubePoints[0].y = cubePoints[1].y = ((slope - 3 + 1) / 2.0f) - MAP_BLOCK_DIMS;
        break;
        // W, 26 low, high
        case 5: case 6:
            cubePoints[1].y = cubePoints[5].y = ((slope - 5) / 2.0f) - MAP_BLOCK_DIMS;
            cubePoints[0].y = cubePoints[4].y = ((slope - 5 + 1) / 2.0f) - MAP_BLOCK_DIMS;
        break;
        // E, 26 low, high
        case 7: case 8:
            cubePoints[0].y = cubePoints[4].y = ((slope - 7) / 2.0f) - MAP_BLOCK_DIMS;
            cubePoints[1].y = cubePoints[5].y = ((slope - 7 + 1) / 2.0f) - MAP_BLOCK_DIMS;
        break;
        // N, 7 low - high
        case 9: case 10: case 11: case 12:
        case 13: case 14: case 15: case 16:
            cubePoints[0].y = cubePoints[1].y = ((slope - 9) / 8.0f) - MAP_BLOCK_DIMS;
            cubePoints[4].y = cubePoints[5].y = ((slope - 9 + 1) / 8.0f) - MAP_BLOCK_DIMS;
        break;
        // S, 7 low - high
        case 17: case 18: case 19: case 20:
        case 21: case 22: case 23: case 24:
            cubePoints[4].y = cubePoints[5].y = ((slope - 17) / 8.0f) - MAP_BLOCK_DIMS;
            cubePoints[0].y = cubePoints[1].y = ((slope - 17 + 1) / 8.0f) - MAP_BLOCK_DIMS;
        break;
        // W, 7 low - high
        case 25: case 26: case 27: case 28:
        case 29: case 30: case 31: case 32:
            cubePoints[1].y = cubePoints[5].y = ((slope - 25) / 8.0f) - MAP_BLOCK_DIMS;
            cubePoints[0].y = cubePoints[4].y = ((slope - 25 + 1) / 8.0f) - MAP_BLOCK_DIMS;
        break;
        // E, 7 low - high
        case 33: case 34: case 35: case 36:
        case 37: case 38: case 39: case 40:
            cubePoints[0].y = cubePoints[4].y = ((slope - 33) / 8.0f) - MAP_BLOCK_DIMS;
            cubePoints[1].y = cubePoints[5].y = ((slope - 33 + 1) / 8.0f) - MAP_BLOCK_DIMS;
        break;
        // 41 - 44 = 45 N,S,W,E
        case 41: cubePoints[0].y = cubePoints[1].y = - MAP_BLOCK_DIMS; break;
        case 42: cubePoints[4].y = cubePoints[5].y = - MAP_BLOCK_DIMS; break;
        case 43: cubePoints[1].y = cubePoints[5].y = - MAP_BLOCK_DIMS; break;
        case 44: cubePoints[0].y = cubePoints[4].y = - MAP_BLOCK_DIMS; break;
    }

    const int rotateLid = (face == eBlockFace_Lid) ? blockInfo->mLidRotation : 0;
    const int baseVertexIndex = meshData.mMeshVertices.size();
    meshData.mMeshVertices.resize(baseVertexIndex + 4);
    meshData.mMeshVertices[baseVertexIndex + ((rotateLid + 0) % 4)].mTexcoord = {0.0f, 0.0f, blockTexIndex * 1.0f};
    meshData.mMeshVertices[baseVertexIndex + ((rotateLid + 1) % 4)].mTexcoord = {1.0f, 0.0f, blockTexIndex * 1.0f};
    meshData.mMeshVertices[baseVertexIndex + ((rotateLid + 2) % 4)].mTexcoord = {1.0f, 1.0f, blockTexIndex * 1.0f};
    meshData.mMeshVertices[baseVertexIndex + ((rotateLid + 3) % 4)].mTexcoord = {0.0f, 1.0f, blockTexIndex * 1.0f};

    unsigned char color = 50 + static_cast<unsigned char>(((posz * 1.0f) / MAP_LAYERS_COUNT) * 180);

    // color
    meshData.mMeshVertices[baseVertexIndex + 0].mColor = MAKE_RGBA(color, color, color, blockInfo->mIsFlat ? 0 : 255);
    meshData.mMeshVertices[baseVertexIndex + 1].mColor = MAKE_RGBA(color, color, color, blockInfo->mIsFlat ? 0 : 255);
    meshData.mMeshVertices[baseVertexIndex + 2].mColor = MAKE_RGBA(color, color, color, blockInfo->mIsFlat ? 0 : 255);
    meshData.mMeshVertices[baseVertexIndex + 3].mColor = MAKE_RGBA(color, color, color, blockInfo->mIsFlat ? 0 : 255);

    // setup face vertices
    const glm::vec3 cubeOffset { posx * MAP_BLOCK_DIMS, posz * MAP_BLOCK_DIMS, posy * MAP_BLOCK_DIMS };
    if (face == eBlockFace_Lid)
    {
        meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[4] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[5] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[1] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[0] + cubeOffset;
    }
    if (face == eBlockFace_S)
    {
        meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[0] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[1] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[2] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[3] + cubeOffset;
    }
    if (face == eBlockFace_N)
    {
        meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[5] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[4] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[7] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[6] + cubeOffset;
    }
    if (face == eBlockFace_W)
    {
        meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[4] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[0] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[3] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[7] + cubeOffset;
    }
    if (face == eBlockFace_E)
    {
        meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[1] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[5] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[6] + cubeOffset;
        meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[2] + cubeOffset;
    }

    if (blockInfo->mIsFlat)
    {
        // should draw at W position
        if (face == eBlockFace_E)
        {
            meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[0] + cubeOffset;
            meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[4] + cubeOffset;
            meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[7] + cubeOffset;
            meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[3] + cubeOffset;
        }
        // should draw at N position
        if (face == eBlockFace_S)
        {
            meshData.mMeshVertices[baseVertexIndex + 0].mPosition = cubePoints[4] + cubeOffset;
            meshData.mMeshVertices[baseVertexIndex + 1].mPosition = cubePoints[5] + cubeOffset;
            meshData.mMeshVertices[baseVertexIndex + 2].mPosition = cubePoints[6] + cubeOffset;
            meshData.mMeshVertices[baseVertexIndex + 3].mPosition = cubePoints[7] + cubeOffset;
        }
    }
    // add indices
    int baseIndex = meshData.mMeshIndices.size();
    meshData.mMeshIndices.resize(baseIndex + 6);
    meshData.mMeshIndices[baseIndex + 0] = baseVertexIndex + 3;
    meshData.mMeshIndices[baseIndex + 1] = baseVertexIndex + 1;
    meshData.mMeshIndices[baseIndex + 2] = baseVertexIndex + 0;
    meshData.mMeshIndices[baseIndex + 3] = baseVertexIndex + 3;
    meshData.mMeshIndices[baseIndex + 4] = baseVertexIndex + 2;
    meshData.mMeshIndices[baseIndex + 5] = baseVertexIndex + 1;
}
