#include "stdafx.h"
#include "SpriteCache.h"
#include "GpuTextureArray2D.h"
#include "GraphicsDevice.h"
#include "CarnageGame.h"
#include "stb_rect_pack.h"

const int ObjectsTextureSizeX = 512;
const int ObjectsTextureSizeY = 512;
const int ObjectsTextureMaxLayers = 8;
const int SpritesSpacing = 4;

SpriteCache gSpriteCache;

void SpriteCache::Cleanup()
{
    FreeBlocksSpritesheet();
    FreeObjectsSpritesheet();
}

bool SpriteCache::CreateObjectsSpritesheet()
{
    CityStyleData& styleData = gCarnageGame.mCityScape.mStyleData; // todo

    debug_assert(styleData.IsLoaded());
    if (mObjectsSpritesheet)
    {
        debug_assert(false);
        return true;
    }

    int totalSprites = styleData.mSprites.size();
    debug_assert(totalSprites > 0);
    if (totalSprites == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Skip building objects atlas");
        return true;
    }

    mObjectsSpritesheet = CreateSpritesheet(ObjectsTextureSizeX, ObjectsTextureSizeY, ObjectsTextureMaxLayers);
    if (mObjectsSpritesheet == nullptr)
    {
        debug_assert(false);
        return false;
    }

    mObjectsSpritesheet->mEtries.resize(totalSprites);

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
        stbrp_rects[icurr].w = styleData.mSprites[isprite].mWidth + SpritesSpacing;
        stbrp_rects[icurr].h = styleData.mSprites[isprite].mHeight + SpritesSpacing;
        stbrp_rects[icurr].was_packed = 0;
        ++icurr;
    }

    float tcx = 1.0f / ObjectsTextureSizeX;
    float tcy = 1.0f / ObjectsTextureSizeY;

    for (int currentLayerIndex = 0; ; ++currentLayerIndex)
    {
        if (currentLayerIndex == ObjectsTextureMaxLayers)
        {
            debug_assert(false);
            return false;
        }

		stbrp_context context;
		stbrp_init_target(&context, ObjectsTextureSizeX, ObjectsTextureSizeY, stbrp_nodes.data(), stbrp_nodes.size());
		bool all_done = stbrp_pack_rects(&context, stbrp_rects.data(), stbrp_rects.size()) > 0;

        // write sprites to temporary bitmap
        int numPacked = 0;
        for (const stbrp_rect& curr_rc: stbrp_rects)
        {
            if (curr_rc.was_packed == 0)
                continue;

            ++numPacked;
            if (!styleData.GetSpriteTexture(curr_rc.id, &spritesBitmap, curr_rc.x, curr_rc.y))
            {
                debug_assert(false);
                return false;
            }

            SpritesheetEntry& spritesheetRecord = mObjectsSpritesheet->mEtries[curr_rc.id];
            spritesheetRecord.mRectangle.x = curr_rc.x;
            spritesheetRecord.mRectangle.y = curr_rc.y;
            spritesheetRecord.mRectangle.w = curr_rc.w - SpritesSpacing;
            spritesheetRecord.mRectangle.h = curr_rc.h - SpritesSpacing;
            spritesheetRecord.mU0 = spritesheetRecord.mRectangle.x * tcx;
            spritesheetRecord.mV0 = spritesheetRecord.mRectangle.y * tcy;
            spritesheetRecord.mU1 = (spritesheetRecord.mRectangle.x + spritesheetRecord.mRectangle.w) * tcx;
            spritesheetRecord.mV1 = (spritesheetRecord.mRectangle.y + spritesheetRecord.mRectangle.h) * tcy;
            spritesheetRecord.mTcZ = currentLayerIndex * 1.0f;
        }

        if (numPacked == 0)
        {
            debug_assert(false);
            return false;
        }

        // upload to texture
        if (!mObjectsSpritesheet->mSpritesheetTexture->Upload(currentLayerIndex, 1, spritesBitmap.mData))
        {
            debug_assert(false);
        }

        if (!all_done)
        {
            stbrp_rects.erase(std::remove_if(stbrp_rects.begin(), stbrp_rects.end(), [](const stbrp_rect& rc)
                {
                    return rc.was_packed == 1;
                }), 
                stbrp_rects.end());
        }

        if (all_done)
            break;

        // clear temporary bitmap before next pack operation
        spritesBitmap.FillWithColor(MAKE_RGBA(255, 255, 255, 0));
    }

    return true;
}

bool SpriteCache::CreateBlocksSpritesheet()
{
    CityStyleData& styleData = gCarnageGame.mCityScape.mStyleData; // todo

    debug_assert(styleData.IsLoaded());
    debug_assert(mBlocksSpritesheet == nullptr);
    if (mBlocksSpritesheet)
        return true;

    // count textures
    const int totalTextures = styleData.GetBlockTexturesCount();
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

    mBlocksSpritesheet = CreateSpritesheet(blockBitmap.mSizex, blockBitmap.mSizey, totalTextures);
    if (mBlocksSpritesheet == nullptr)
    {
        debug_assert(mBlocksSpritesheet);
        return false;
    }
    
    int currentLayerIndex = 0;
    for (int recordIndex = 0, iblockType = 0; iblockType < eBlockType_COUNT; ++iblockType)
    {
        int numTextures = styleData.GetBlockTexturesCount((eBlockType) iblockType);
        for (int itexture = 0; itexture < numTextures; ++itexture)
        {
            if (!styleData.GetBlockTexture((eBlockType) iblockType, itexture, &blockBitmap, 0, 0))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Cannot read block texture: %d %d", iblockType, itexture);
                return false;
            }

            // setup spritesheet record
            SpritesheetEntry& spritesheetRecord = mBlocksSpritesheet->mEtries[recordIndex++];
            spritesheetRecord.mRectangle.x = 0;
            spritesheetRecord.mRectangle.y = 0;
            spritesheetRecord.mRectangle.w = MAP_BLOCK_TEXTURE_DIMS;
            spritesheetRecord.mRectangle.h = MAP_BLOCK_TEXTURE_DIMS;
            spritesheetRecord.mU0 = 0.0f;
            spritesheetRecord.mV0 = 0.0f;
            spritesheetRecord.mU1 = 1.0f;
            spritesheetRecord.mV1 = 1.0f;
            spritesheetRecord.mTcZ = currentLayerIndex * 1.0f;

            // upload bitmap to gpu
            if (!mBlocksSpritesheet->mSpritesheetTexture->Upload(currentLayerIndex, 1, blockBitmap.mData))
            {
                debug_assert(false);
            }
            
            ++currentLayerIndex;
        }
    }
    return true;
}

bool SpriteCache::GetBlockSpritesheetEntry(eBlockType blockType, int blockIndex, SpritesheetEntry& spritesheetEntry) const
{
    CityStyleData& styleData = gCarnageGame.mCityScape.mStyleData; // todo

    debug_assert(mBlocksSpritesheet);
    debug_assert(styleData.IsLoaded());
    if (mBlocksSpritesheet == nullptr)
        return false;

    const int linearIndex = styleData.GetBlockTextureLinearIndex(blockType, blockIndex);
    const int numRecords = mBlocksSpritesheet->mEtries.size();
    assert(linearIndex < numRecords);

    if (linearIndex < numRecords)
    {
        spritesheetEntry = mBlocksSpritesheet->mEtries[linearIndex];
    }
    return linearIndex < numRecords;
}

Spritesheet* SpriteCache::CreateSpritesheet(int picSizex, int picSizey, int picsCount)
{
    if (picSizex < 1 || picSizey < 1 || picsCount < 1)
    {
        debug_assert(false);
        return nullptr;
    }

    Spritesheet* spritesheet = new Spritesheet;
    spritesheet->mSpritesheetTexture = gGraphicsDevice.CreateTextureArray2D(eTextureFormat_RGBA8, picSizex, picSizey, picsCount, nullptr);
    debug_assert(spritesheet->mSpritesheetTexture);

    if (spritesheet->mSpritesheetTexture == nullptr)
    {
        SafeDelete(spritesheet);
        return nullptr;
    }
    spritesheet->mEtries.resize(picsCount);
    return spritesheet;
}

void SpriteCache::FreeSpritesheet(Spritesheet* spritesheet)
{
    debug_assert(spritesheet);
    if (spritesheet)
    {
        debug_assert(spritesheet->mSpritesheetTexture);
        if (spritesheet->mSpritesheetTexture)
        {
            gGraphicsDevice.DestroyTexture(spritesheet->mSpritesheetTexture);
            spritesheet->mSpritesheetTexture = nullptr;
        }
        SafeDelete(spritesheet);
    }
}

void SpriteCache::FreeBlocksSpritesheet()
{
    if (mBlocksSpritesheet)
    {
        FreeSpritesheet(mBlocksSpritesheet);
        mBlocksSpritesheet = nullptr;
    }
}

void SpriteCache::FreeObjectsSpritesheet()
{
    if (mObjectsSpritesheet)
    {
        FreeSpritesheet(mObjectsSpritesheet);
        mObjectsSpritesheet = nullptr;
    }
}