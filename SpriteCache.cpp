#include "stdafx.h"
#include "SpriteCache.h"
#include "GpuTextureArray2D.h"
#include "GraphicsDevice.h"
#include "CarnageGame.h"

SpriteCache gSpriteCache;

void SpriteCache::Cleanup()
{
    if (mBlocksSpritesheet)
    {
        FreeSpritesheet(mBlocksSpritesheet);
        mBlocksSpritesheet = nullptr;
    }
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
