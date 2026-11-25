#include "stdafx.h"
#include "GameMapHelpers.h"
#include "SpriteManager.h"
#include "GameMapManager.h"

bool GameMapHelpers::BuildMapMesh(GameMapManager& cityScape, const Rect& area, int layerIndex, CityMeshData& meshData)
{
    debug_assert(layerIndex > -1 && layerIndex < MAP_LAYERS_COUNT);

    // preallocate
    meshData.mBlocksIndices.reserve(1 * 1024 * 1024);
    meshData.mBlocksVertices.reserve(1 * 1024 * 1024);

    // prepare
    for (int tiley = 0; tiley < area.h; ++tiley)
    for (int tilex = 0; tilex < area.w; ++tilex)
    {
        const MapBlockInfo* mapBlock = cityScape.GetBlockInfo(tilex + area.x, tiley + area.y, layerIndex);
        if (mapBlock == nullptr)
            continue;

        for (int iface = 0; iface < eBlockFace_COUNT; ++iface)
        {
            if (mapBlock->mFaces[iface] == 0)
                continue;

            eBlockFace faceid = (eBlockFace) iface;
            PutBlockFace(cityScape, meshData, tilex + area.x, tiley + area.y, layerIndex, faceid, mapBlock);
        }
    }
    return true;
}

bool GameMapHelpers::BuildMapMesh(GameMapManager& cityScape, const Rect& area, CityMeshData& meshData)
{
    // preallocate
    meshData.mBlocksIndices.reserve(4 * 1024 * 1024);
    meshData.mBlocksVertices.reserve(4 * 1024 * 1024);

    // prepare
    for (int tilez = 0; tilez < MAP_LAYERS_COUNT; ++tilez)
    for (int tiley = 0; tiley < area.h; ++tiley)
    for (int tilex = 0; tilex < area.w; ++tilex)
    {
        const MapBlockInfo* mapBlock = cityScape.GetBlockInfo(tilex + area.x, tiley + area.y, tilez);
        if (mapBlock == nullptr)
            continue;

        for (int iface = 0; iface < eBlockFace_COUNT; ++iface)
        {
            if (mapBlock->mFaces[iface] == 0)
                continue;

            eBlockFace faceid = (eBlockFace) iface;
            PutBlockFace(cityScape, meshData, tilex + area.x, tiley + area.y, tilez, faceid, mapBlock);
        }
    }
    return true;
}

void GameMapHelpers::PutBlockFace(GameMapManager& cityScape, CityMeshData& meshData, int x, int y, int z, eBlockFace face, const MapBlockInfo* blockInfo)
{
    assert(blockInfo && blockInfo->mFaces[face]);
    eBlockType blockType = (face == eBlockFace_Lid) ? eBlockType_Lid : eBlockType_Side;

    const int blockTexIndex = cityScape.mStyleData.GetBlockTextureLinearIndex(blockType, blockInfo->mFaces[face]);

    // setup base cube vertices
    glm::vec3 cubePoints[] =
    {
        // front face, cw
        { 0.0f, 1.0f, 1.0f }, 
        { 1.0f, 1.0f, 1.0f }, 
        { 1.0f, 0.0f, 1.0f }, 
        { 0.0f, 0.0f, 1.0f },
        // back face, cw
        { 0.0f, 1.0f, 0.0f }, 
        { 1.0f, 1.0f, 0.0f }, 
        { 1.0f, 0.0f, 0.0f }, 
        { 0.0f, 0.0f, 0.0f },
    };

    // scale to meters
    for (glm::vec3& currPoint: cubePoints)
    {
        currPoint *= METERS_PER_MAP_UNIT;
    }

    glm::vec3 texCoords[4] =
    {
        {0.0f, 0.0f, blockTexIndex * 1.0f},
        {1.0f, 0.0f, blockTexIndex * 1.0f},
        {1.0f, 1.0f, blockTexIndex * 1.0f},
        {0.0f, 1.0f, blockTexIndex * 1.0f}
    };

    // process slope
    const int slope = blockInfo->mSlopeType;
    switch (slope)
    {
        // N, 26 low, high
        case 1: case 2:
            cubePoints[0].y = cubePoints[1].y = ((slope - 1) / 2.0f) * METERS_PER_MAP_UNIT;
            cubePoints[4].y = cubePoints[5].y = ((slope - 1 + 1) / 2.0f) * METERS_PER_MAP_UNIT;
        break;
        // S, 26 low, high
        case 3: case 4:
            cubePoints[4].y = cubePoints[5].y = ((slope - 3) / 2.0f) * METERS_PER_MAP_UNIT;
            cubePoints[0].y = cubePoints[1].y = ((slope - 3 + 1) / 2.0f) * METERS_PER_MAP_UNIT;
        break;
        // W, 26 low, high
        case 5: case 6:
            cubePoints[1].y = cubePoints[5].y = ((slope - 5) / 2.0f) * METERS_PER_MAP_UNIT;
            cubePoints[0].y = cubePoints[4].y = ((slope - 5 + 1) / 2.0f) * METERS_PER_MAP_UNIT;
        break;
        // E, 26 low, high
        case 7: case 8:
            cubePoints[0].y = cubePoints[4].y = ((slope - 7) / 2.0f) * METERS_PER_MAP_UNIT;
            cubePoints[1].y = cubePoints[5].y = ((slope - 7 + 1) / 2.0f) * METERS_PER_MAP_UNIT;
        break;
        // N, 7 low - high
        case 9: case 10: case 11: case 12:
        case 13: case 14: case 15: case 16:
            cubePoints[0].y = cubePoints[1].y = ((slope - 9) / 8.0f) * METERS_PER_MAP_UNIT;
            cubePoints[4].y = cubePoints[5].y = ((slope - 9 + 1) / 8.0f) * METERS_PER_MAP_UNIT;
        break;
        // S, 7 low - high
        case 17: case 18: case 19: case 20:
        case 21: case 22: case 23: case 24:
            cubePoints[4].y = cubePoints[5].y = ((slope - 17) / 8.0f) * METERS_PER_MAP_UNIT;
            cubePoints[0].y = cubePoints[1].y = ((slope - 17 + 1) / 8.0f) * METERS_PER_MAP_UNIT;
        break;
        // W, 7 low - high
        case 25: case 26: case 27: case 28:
        case 29: case 30: case 31: case 32:
            cubePoints[1].y = cubePoints[5].y = ((slope - 25) / 8.0f) * METERS_PER_MAP_UNIT;
            cubePoints[0].y = cubePoints[4].y = ((slope - 25 + 1) / 8.0f) * METERS_PER_MAP_UNIT;
        break;
        // E, 7 low - high
        case 33: case 34: case 35: case 36:
        case 37: case 38: case 39: case 40:
            cubePoints[0].y = cubePoints[4].y = ((slope - 33) / 8.0f) * METERS_PER_MAP_UNIT;
            cubePoints[1].y = cubePoints[5].y = ((slope - 33 + 1) / 8.0f) * METERS_PER_MAP_UNIT;
        break;
        // 41 - 44 = 45 N,S,W,E
        case 41: cubePoints[0].y = cubePoints[1].y = 0.0f; break;
        case 42: cubePoints[4].y = cubePoints[5].y = 0.0f; break;
        case 43: cubePoints[1].y = cubePoints[5].y = 0.0f; break;
        case 44: cubePoints[0].y = cubePoints[4].y = 0.0f; break;
    }

    const int rotateLid = (face == eBlockFace_Lid) ? blockInfo->mLidRotation : 0;
    const int baseVertexIndex = meshData.mBlocksVertices.size();
    meshData.mBlocksVertices.resize(baseVertexIndex + 4);
    meshData.mBlocksVertices[baseVertexIndex + ((rotateLid + 0) % 4)].mTexcoord = texCoords[0];
    meshData.mBlocksVertices[baseVertexIndex + ((rotateLid + 1) % 4)].mTexcoord = texCoords[1];
    meshData.mBlocksVertices[baseVertexIndex + ((rotateLid + 2) % 4)].mTexcoord = texCoords[2];
    meshData.mBlocksVertices[baseVertexIndex + ((rotateLid + 3) % 4)].mTexcoord = texCoords[3];

    if (face != eBlockFace_Lid)
    {
        bool flipLeftRightFaces = ((blockInfo->mIsFlat != blockInfo->mFlipLeftRightFaces) && (face == eBlockFace_E)) ||
            (blockInfo->mFlipLeftRightFaces && (face == eBlockFace_W));

        if (flipLeftRightFaces)
        {
            std::swap(meshData.mBlocksVertices[baseVertexIndex + 0].mTexcoord, meshData.mBlocksVertices[baseVertexIndex + 1].mTexcoord);
            std::swap(meshData.mBlocksVertices[baseVertexIndex + 2].mTexcoord, meshData.mBlocksVertices[baseVertexIndex + 3].mTexcoord);
        }

        bool flipTopBottomFaces = ((blockInfo->mIsFlat != blockInfo->mFlipTopBottomFaces) && (face == eBlockFace_S)) ||
            (blockInfo->mFlipTopBottomFaces && (face == eBlockFace_N));

        if (flipTopBottomFaces)
        {
            std::swap(meshData.mBlocksVertices[baseVertexIndex + 0].mTexcoord, meshData.mBlocksVertices[baseVertexIndex + 1].mTexcoord);
            std::swap(meshData.mBlocksVertices[baseVertexIndex + 2].mTexcoord, meshData.mBlocksVertices[baseVertexIndex + 3].mTexcoord);
        }
    }

    // color
    int remap = (face == eBlockFace_Lid) ? blockInfo->mRemap : 0;
    meshData.mBlocksVertices[baseVertexIndex + 0].SetColorData(remap, blockInfo->mIsFlat ? 1 : 0);
    meshData.mBlocksVertices[baseVertexIndex + 1].SetColorData(remap, blockInfo->mIsFlat ? 1 : 0);
    meshData.mBlocksVertices[baseVertexIndex + 2].SetColorData(remap, blockInfo->mIsFlat ? 1 : 0);
    meshData.mBlocksVertices[baseVertexIndex + 3].SetColorData(remap, blockInfo->mIsFlat ? 1 : 0);

    // setup face vertices
    glm::vec3 cubeOffset { x * METERS_PER_MAP_UNIT, z * METERS_PER_MAP_UNIT, y * METERS_PER_MAP_UNIT };
    if (face == eBlockFace_Lid)
    {
        meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[4] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[5] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[1] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[0] + cubeOffset;
    }
    if (face == eBlockFace_S)
    {
        meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[0] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[1] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[2] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[3] + cubeOffset;
    }
    if (face == eBlockFace_N)
    {
        meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[5] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[4] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[7] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[6] + cubeOffset;
    }
    if (face == eBlockFace_W)
    {
        meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[4] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[0] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[3] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[7] + cubeOffset;
    }
    if (face == eBlockFace_E)
    {
        meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[1] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[5] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[6] + cubeOffset;
        meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[2] + cubeOffset;
    }

    if (blockInfo->mIsFlat)
    {
        // should draw at W position
        if (face == eBlockFace_E)
        {
            meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[0] + cubeOffset;
            meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[4] + cubeOffset;
            meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[7] + cubeOffset;
            meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[3] + cubeOffset;
        }
        // should draw at N position
        if (face == eBlockFace_S)
        {
            meshData.mBlocksVertices[baseVertexIndex + 0].mPosition = cubePoints[4] + cubeOffset;
            meshData.mBlocksVertices[baseVertexIndex + 1].mPosition = cubePoints[5] + cubeOffset;
            meshData.mBlocksVertices[baseVertexIndex + 2].mPosition = cubePoints[6] + cubeOffset;
            meshData.mBlocksVertices[baseVertexIndex + 3].mPosition = cubePoints[7] + cubeOffset;
        }
    }

    // add indices
    int baseIndex = meshData.mBlocksIndices.size();
    meshData.mBlocksIndices.resize(baseIndex + 6);
    meshData.mBlocksIndices[baseIndex + 0] = baseVertexIndex + 3;
    meshData.mBlocksIndices[baseIndex + 1] = baseVertexIndex + 1;
    meshData.mBlocksIndices[baseIndex + 2] = baseVertexIndex + 0;
    meshData.mBlocksIndices[baseIndex + 3] = baseVertexIndex + 3;
    meshData.mBlocksIndices[baseIndex + 4] = baseVertexIndex + 2;
    meshData.mBlocksIndices[baseIndex + 5] = baseVertexIndex + 1;
}

float GameMapHelpers::GetSlopeHeight(int slopeType, float coord_x, float coord_y)
{
    debug_assert(coord_x >= 0.0f && coord_x <= 1.0f);
    debug_assert(coord_y >= 0.0f && coord_y <= 1.0f);

    const int px = glm::clamp((int) (coord_x * PIXELS_PER_MAP_UNIT), 0, PIXELS_PER_MAP_UNIT - 1);
    const int py = glm::clamp((int) (coord_y * PIXELS_PER_MAP_UNIT), 0, PIXELS_PER_MAP_UNIT - 1);

    int heightPixels = 0;
    switch (slopeType)
    {
        // 26 degrees: N, S, W, E, each in 2 blocks
        case 1: heightPixels = 31 - py / 2; break;
        case 2: heightPixels = 63 - py / 2; break;
        case 3: heightPixels = py / 2; break;
        case 4: heightPixels = 32 + py / 2; break;
        case 5: heightPixels = 31 - px / 2; break;
        case 6: heightPixels = 63 - px / 2; break;
        case 7: heightPixels = px / 2; break;
        case 8: heightPixels = 32 + px / 2; break;
        // 7 degrees: N, S, W, E, each in 8 blocks, lowest block starts 2 pixels up
        case 9: heightPixels = 9 - py / 8; break;
        case 10: case 11: case 12: case 13: case 14: case 15: case 16:
            heightPixels = (slopeType - 9) * 8 + 7 - py / 8;
        break;
        case 17: heightPixels = 2 + py / 8; break;
        case 18: case 19: case 20: case 21: case 22: case 23: case 24:
            heightPixels = (slopeType - 17) * 8 + py / 8;
        break;
        case 25: heightPixels = 9 - px / 8; break;
        case 26: case 27: case 28: case 29: case 30: case 31: case 32:
            heightPixels = (slopeType - 25) * 8 + 7 - px / 8;
        break;
        case 33: heightPixels = 2 + px / 8; break;
        case 34: case 35: case 36: case 37: case 38: case 39: case 40:
            heightPixels = (slopeType - 33) * 8 + px / 8;
        break;
        // 45 degrees: N, S, W, E
        case 41: heightPixels = 63 - py; break;
        case 42: heightPixels = py; break;
        case 43: heightPixels = 63 - px; break;
        case 44: heightPixels = px; break;
        case 0: break;
        default:
            debug_assert(false);
        break;
    }
    return Convert::PixelsToMapUnits(heightPixels);
}
