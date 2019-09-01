#include "stdafx.h"
#include "SpriteCache.h"
#include "GpuTextureArray2D.h"
#include "GpuTexture2D.h"
#include "GraphicsDevice.h"
#include "CarnageGame.h"
#include "stb_rect_pack.h"

const int ObjectsTextureSizeX = 2048;
const int ObjectsTextureSizeY = 1024;
const int SpritesSpacing = 4;

SpriteCache gSpriteCache;


bool SpriteCache::InitLevelSprites(CityStyleData& cityStyle)
{
    Cleanup();

    debug_assert(cityStyle.IsLoaded());

    if (!InitBlocksTexture(cityStyle))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create blocks texture");
        return false;
    }

    if (!InitObjectsSpritesheet(cityStyle))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create objects spritesheet");
        return false;
    }
    return true;
}

void SpriteCache::Cleanup()
{
    if (mBlocksTextureArray2D)
    {
        gGraphicsDevice.DestroyTexture(mBlocksTextureArray2D);
        mBlocksTextureArray2D = nullptr;
    }

    if (mObjectsSpritesheet.mSpritesheetTexture)
    {
        gGraphicsDevice.DestroyTexture(mObjectsSpritesheet.mSpritesheetTexture);
        mObjectsSpritesheet.mSpritesheetTexture = nullptr;
    }
    mObjectsSpritesheet.mEtries.clear();
}

bool SpriteCache::InitObjectsSpritesheet(CityStyleData& cityStyle)
{
    int totalSprites = cityStyle.mSprites.size();
    debug_assert(totalSprites > 0);
    if (totalSprites == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Skip building objects atlas");
        return true;
    }

    debug_assert(ObjectsTextureSizeX > 0);
    debug_assert(ObjectsTextureSizeY > 0);

    mObjectsSpritesheet.mSpritesheetTexture = gGraphicsDevice.CreateTexture2D(eTextureFormat_RGBA8, ObjectsTextureSizeX, ObjectsTextureSizeY, nullptr);
    debug_assert(mObjectsSpritesheet.mSpritesheetTexture);

    if (mObjectsSpritesheet.mSpritesheetTexture == nullptr)
        return false;

    mObjectsSpritesheet.mEtries.resize(totalSprites);

    // allocate temporary bitmap
    PixelsArray spritesBitmap;
    if (!spritesBitmap.Create(eTextureFormat_RGBA8, ObjectsTextureSizeX, ObjectsTextureSizeY))
    {
        debug_assert(false);
        return true;
    }

    spritesBitmap.FillWithColor(MAKE_RGBA(255, 255, 255, 0));

    // detect total layers count
    std::vector<stbrp_node> stbrp_nodes(ObjectsTextureSizeX);
    std::vector<stbrp_rect> stbrp_rects(totalSprites);

    // prepare sprites
    for (int isprite = 0, icurr = 0; isprite < totalSprites; ++isprite)
    {
        stbrp_rects[icurr].id = isprite;
        stbrp_rects[icurr].w = cityStyle.mSprites[isprite].mWidth + SpritesSpacing;
        stbrp_rects[icurr].h = cityStyle.mSprites[isprite].mHeight + SpritesSpacing;
        stbrp_rects[icurr].was_packed = 0;
        ++icurr;
    }

    float tcx = 1.0f / ObjectsTextureSizeX;
    float tcy = 1.0f / ObjectsTextureSizeY;

    // pack sprites
    bool all_done = false;
    {
		stbrp_context context;
		stbrp_init_target(&context, ObjectsTextureSizeX, ObjectsTextureSizeY, stbrp_nodes.data(), stbrp_nodes.size());
		all_done = stbrp_pack_rects(&context, stbrp_rects.data(), stbrp_rects.size()) > 0;

        // write sprites to temporary bitmap
        int numPacked = 0;
        for (const stbrp_rect& curr_rc: stbrp_rects)
        {
            if (curr_rc.was_packed == 0)
                continue;

            ++numPacked;
            if (!cityStyle.GetSpriteTexture(curr_rc.id, &spritesBitmap, curr_rc.x, curr_rc.y))
            {
                debug_assert(false);
                return false;
            }

            SpritesheetEntry& spritesheetRecord = mObjectsSpritesheet.mEtries[curr_rc.id];
            spritesheetRecord.mRectangle.x = curr_rc.x;
            spritesheetRecord.mRectangle.y = curr_rc.y;
            spritesheetRecord.mRectangle.w = curr_rc.w - SpritesSpacing;
            spritesheetRecord.mRectangle.h = curr_rc.h - SpritesSpacing;
            spritesheetRecord.mU0 = spritesheetRecord.mRectangle.x * tcx;
            spritesheetRecord.mV0 = spritesheetRecord.mRectangle.y * tcy;
            spritesheetRecord.mU1 = (spritesheetRecord.mRectangle.x + spritesheetRecord.mRectangle.w) * tcx;
            spritesheetRecord.mV1 = (spritesheetRecord.mRectangle.y + spritesheetRecord.mRectangle.h) * tcy;
        }

        if (numPacked == 0)
        {
            debug_assert(false);
            return false;
        }

        // upload to texture
        if (!mObjectsSpritesheet.mSpritesheetTexture->Upload(spritesBitmap.mData))
        {
            debug_assert(false);
        }
    }
    debug_assert(all_done);
    return all_done;
}

bool SpriteCache::InitBlocksTexture(CityStyleData& cityStyle)
{
    // count textures
    const int totalTextures = cityStyle.GetBlockTexturesCount();
    assert(totalTextures > 0);
    if (totalTextures == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Skip building blocks atlas");
        return true;
    }

    // allocate temporary bitmap
    PixelsArray blockBitmap;
    if (!blockBitmap.Create(eTextureFormat_RGBA8, MAP_BLOCK_TEXTURE_DIMS, MAP_BLOCK_TEXTURE_DIMS))
    {
        debug_assert(false);
        return true;
    }

    mBlocksTextureArray2D = gGraphicsDevice.CreateTextureArray2D(eTextureFormat_RGBA8, blockBitmap.mSizex, blockBitmap.mSizey, totalTextures, nullptr);
    debug_assert(mBlocksTextureArray2D);
    
    int currentLayerIndex = 0;
    for (int recordIndex = 0, iblockType = 0; iblockType < eBlockType_COUNT; ++iblockType)
    {
        int numTextures = cityStyle.GetBlockTexturesCount((eBlockType) iblockType);
        for (int itexture = 0; itexture < numTextures; ++itexture)
        {
            if (!cityStyle.GetBlockTexture((eBlockType) iblockType, itexture, &blockBitmap, 0, 0))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Cannot read block texture: %d %d", iblockType, itexture);
                return false;
            }

            // upload bitmap to gpu
            if (!mBlocksTextureArray2D->Upload(currentLayerIndex, 1, blockBitmap.mData))
            {
                debug_assert(false);
            }
            
            ++currentLayerIndex;
        }
    }
    return true;
}