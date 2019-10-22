#include "stdafx.h"
#include "SpriteManager.h"
#include "GpuTextureArray2D.h"
#include "GpuTexture2D.h"
#include "GraphicsDevice.h"
#include "CarnageGame.h"
#include "stb_rect_pack.h"
#include "GpuBufferTexture.h"
#include "GameCheatsWindow.h"
#include "MemoryManager.h"

const int ObjectsTextureSizeX = 2048;
const int ObjectsTextureSizeY = 1024;
const int SpritesSpacing = 4;

SpriteManager gSpriteManager;

bool SpriteManager::InitLevelSprites()
{
    Cleanup();
    debug_assert(gGameMap.mStyleData.IsLoaded());

    if (!InitBlocksTexture())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create blocks texture");
        return false;
    }

    if (!InitBlocksIndicesTable())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize blocks indices table texture");
        return false;
    }

    if (!InitObjectsSpritesheet())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create objects spritesheet");
        return false;
    }
    InitBlocksAnimations();
    return true;
}

void SpriteManager::Cleanup()
{
    FlushSpritesCache();
    DestroySpriteTextures();
    mIndicesTableChanged = false;
    if (mBlocksTextureArray)
    {
        gGraphicsDevice.DestroyTexture(mBlocksTextureArray);
        mBlocksTextureArray = nullptr;
    }

    if (mBlocksIndicesTable)
    {
        gGraphicsDevice.DestroyTexture(mBlocksIndicesTable);
        mBlocksIndicesTable = nullptr;
    }

    if (mObjectsSpritesheet.mSpritesheetTexture)
    {
        gGraphicsDevice.DestroyTexture(mObjectsSpritesheet.mSpritesheetTexture);
        mObjectsSpritesheet.mSpritesheetTexture = nullptr;
    }

    mBlocksIndices.clear();
    mBlocksAnimations.clear();
    mObjectsSpritesheet.mEntries.clear();
}

bool SpriteManager::InitObjectsSpritesheet()
{
    StyleData& cityStyle = gGameMap.mStyleData;

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

    mObjectsSpritesheet.mEntries.resize(totalSprites);

    // allocate temporary bitmap
    PixelsArray spritesBitmap;
    if (!spritesBitmap.Create(eTextureFormat_RGBA8, ObjectsTextureSizeX, ObjectsTextureSizeY, gMemoryManager.mFrameHeapAllocator))
    {
        debug_assert(false);
        return false;
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

            TextureRegion& spritesheetRecord = mObjectsSpritesheet.mEntries[curr_rc.id];
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

bool SpriteManager::InitBlocksTexture()
{
    StyleData& cityStyle = gGameMap.mStyleData;
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
    if (!blockBitmap.Create(eTextureFormat_RGBA8, MAP_BLOCK_TEXTURE_DIMS, MAP_BLOCK_TEXTURE_DIMS, gMemoryManager.mFrameHeapAllocator))
    {
        debug_assert(false);
        return false;
    }

    mBlocksTextureArray = gGraphicsDevice.CreateTextureArray2D(eTextureFormat_RGBA8, blockBitmap.mSizex, blockBitmap.mSizey, totalTextures, nullptr);
    debug_assert(mBlocksTextureArray);
    
    int currentLayerIndex = 0;
    for (int iblockType = 0; iblockType < eBlockType_COUNT; ++iblockType)
    {
        int numTextures = cityStyle.GetBlockTexturesCount((eBlockType) iblockType);
        for (int itexture = 0; itexture < numTextures; ++itexture)
        {
            if (!cityStyle.GetBlockTexture((eBlockType) iblockType, itexture, &blockBitmap, 0, 0, 0))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Cannot read block texture: %d %d", iblockType, itexture);
                return false;
            }

            // upload bitmap to gpu
            if (!mBlocksTextureArray->Upload(currentLayerIndex, 1, blockBitmap.mData))
            {
                debug_assert(false);
            }
            
            ++currentLayerIndex;
        }
    }
    return true;
}

bool SpriteManager::InitBlocksIndicesTable()
{
    StyleData& cityStyle = gGameMap.mStyleData;

    // count textures
    const int totalTextures = cityStyle.GetBlockTexturesCount();
    assert(totalTextures > 0);
    if (totalTextures == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Skip building blocks indices table");
        return true;
    }

    mBlocksIndices.resize(totalTextures);

    // reset to default order
    for (int i = 0; i < totalTextures; ++i)
    {
        mBlocksIndices[i] = i;
    }

    mBlocksIndicesTable = gGraphicsDevice.CreateBufferTexture(eTextureFormat_R16UI, mBlocksIndices.size() * sizeof(unsigned short), mBlocksIndices.data());
    debug_assert(mBlocksIndicesTable);

    return true;
}

void SpriteManager::RenderFrameBegin()
{

}

void SpriteManager::RenderFrameEnd()
{
    if (mIndicesTableChanged)
    {
        // upload indices table
        debug_assert(mBlocksIndicesTable);
        mIndicesTableChanged = false;
        mBlocksIndicesTable->Upload(0, mBlocksIndices.size() * sizeof(unsigned short), mBlocksIndices.data());
    }
}

void SpriteManager::InitBlocksAnimations()
{
    StyleData& cityStyle = gGameMap.mStyleData;

    mBlocksAnimations.clear();
    for (const BlockAnimationStyle& currAnim: cityStyle.mBlocksAnimations)
    {
        BlockAnimation animData;
        animData.mBlockIndex = cityStyle.GetBlockTextureLinearIndex((currAnim.mWhich == 0 ? eBlockType_Side : eBlockType_Lid), currAnim.mBlock);
        animData.mAnimData.mFramesPerSecond = (20.0f / currAnim.mSpeed);
        animData.mAnimData.mFramesCount = currAnim.mFrameCount + 1;
        animData.mAnimData.mFrames[0] = animData.mBlockIndex; // initial frame
        for (int iframe = 0; iframe < currAnim.mFrameCount; ++iframe)
        {   
            // convert to linear indices
            animData.mAnimData.mFrames[iframe + 1] = cityStyle.GetBlockTextureLinearIndex(eBlockType_Aux, currAnim.mFrames[iframe]);
        }
        animData.PlayAnimation(eSpriteAnimLoop_FromStart);
        mBlocksAnimations.push_back(animData);
    }
}

void SpriteManager::UpdateBlocksAnimations(Timespan deltaTime)
{
    if (!gGameCheatsWindow.mEnableBlocksAnimation)
        return;

    for (BlockAnimation& currAnim: mBlocksAnimations)
    {
        if (!currAnim.AdvanceAnimation(deltaTime))
            continue;
        mBlocksIndices[currAnim.mBlockIndex] = currAnim.GetCurrentFrame(); // patch table
        mIndicesTableChanged = true;
    }
}

void SpriteManager::DumpBlocksTexture(const char* outputLocation)
{
    StyleData& cityStyle = gGameMap.mStyleData;

    debug_assert(cityStyle.IsLoaded());
    cxx::ensure_path_exists(outputLocation);
    // allocate temporary bitmap
    PixelsArray blockBitmap;
    if (!blockBitmap.Create(eTextureFormat_RGBA8, MAP_BLOCK_TEXTURE_DIMS, MAP_BLOCK_TEXTURE_DIMS, gMemoryManager.mFrameHeapAllocator))
    {
        debug_assert(false);
        return;
    }
    cxx::string_buffer_1024 pathBuffer;
    for (int iblockType = 0; iblockType < eBlockType_COUNT; ++iblockType)
    {
        eBlockType currentBlockType = (eBlockType) iblockType;

        int numTextures = cityStyle.GetBlockTexturesCount(currentBlockType);
        for (int itexture = 0; itexture < numTextures; ++itexture)
        {
            if (!cityStyle.GetBlockTexture(currentBlockType, itexture, &blockBitmap, 0, 0, 0))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Cannot read block texture: %d %d", iblockType, itexture);
                continue;
            }
            
            // dump to file
            pathBuffer.printf("%s/%s_%d.png", outputLocation, cxx::enum_to_string(currentBlockType), itexture);
            if (!blockBitmap.SaveToFile(pathBuffer.c_str()))
            {
                debug_assert(false);
            }
        }
    } // for
}

void SpriteManager::DumpSpriteTextures(const char* outputLocation)
{
    StyleData& cityStyle = gGameMap.mStyleData;

    debug_assert(cityStyle.IsLoaded());
    cxx::ensure_path_exists(outputLocation);
    cxx::string_buffer_1024 pathBuffer;
    for (int iSpriteType = 0; iSpriteType < eSpriteType_COUNT; ++iSpriteType)
    {
        eSpriteType sprite_type = (eSpriteType) iSpriteType;
        for (int iSpriteId = 0; iSpriteId < cityStyle.GetNumSprites(sprite_type); ++iSpriteId)
        {
            int sprite_index = cityStyle.GetSpriteIndex(sprite_type, iSpriteId);

            PixelsArray spriteBitmap;
            spriteBitmap.Create(eTextureFormat_RGBA8, 
                cityStyle.mSprites[sprite_index].mWidth, 
                cityStyle.mSprites[sprite_index].mHeight, gMemoryManager.mFrameHeapAllocator);
            cityStyle.GetSpriteTexture(sprite_index, &spriteBitmap, 0, 0);
            
            // dump to file
            pathBuffer.printf("%s/%s_%d.png", outputLocation, cxx::enum_to_string(sprite_type), iSpriteId);
            if (!spriteBitmap.SaveToFile(pathBuffer.c_str()))
            {
                debug_assert(false);
            }
        }
    } // for
}

void SpriteManager::DumpCarsTextures(const char* outputLocation)
{
    StyleData& cityStyle = gGameMap.mStyleData;

    debug_assert(cityStyle.IsLoaded());
    cxx::ensure_path_exists(outputLocation);
    cxx::string_buffer_1024 pathBuffer;
    for (const CarStyle& currCar: cityStyle.mCars)
    {
        int sprite_index = cityStyle.GetCarSpriteIndex(currCar.mVType, currCar.mSprNum);

        PixelsArray spriteBitmap;
        spriteBitmap.Create(eTextureFormat_RGBA8, 
            cityStyle.mSprites[sprite_index].mWidth, 
            cityStyle.mSprites[sprite_index].mHeight, gMemoryManager.mFrameHeapAllocator);
        cityStyle.GetSpriteTexture(sprite_index, &spriteBitmap, 0, 0);
            
        // dump to file
        pathBuffer.printf("%s/%d-%s-%s.png", outputLocation, currCar.mModelId, 
            cxx::enum_to_string(currCar.mModelId), 
            cxx::enum_to_string(currCar.mVType));

        if (!spriteBitmap.SaveToFile(pathBuffer.c_str()))
        {
            debug_assert(false);
        }
    } // for
}

void SpriteManager::DumpSpriteDeltas(const char* outputLocation)
{
    StyleData& cityStyle = gGameMap.mStyleData;

    debug_assert(cityStyle.IsLoaded());
    cxx::ensure_path_exists(outputLocation);
    cxx::string_buffer_1024 pathBuffer;

    for (int isprite = 0, Num = gGameMap.mStyleData.mSprites.size(); isprite < Num; ++isprite)
    {
        SpriteStyle& sprite = gGameMap.mStyleData.mSprites[isprite];

        PixelsArray spriteBitmap;
        spriteBitmap.Create(eTextureFormat_RGBA8, sprite.mWidth, sprite.mHeight, gMemoryManager.mFrameHeapAllocator);
        for (int idelta = 0; idelta < sprite.mDeltaCount; ++idelta)
        {
            if (!cityStyle.GetSpriteTexture(isprite, BIT(idelta), &spriteBitmap, 0, 0))
            {
                debug_assert(false);
            }

            // dump to file
            pathBuffer.printf("%s/sprite_%d_delta_%d.png", outputLocation, isprite, idelta);
            if (!spriteBitmap.SaveToFile(pathBuffer.c_str()))
            {
                debug_assert(false);
            }
        }
    } // for
}

void SpriteManager::DumpSpriteDeltas(const char* outputLocation, int spriteIndex)
{
    StyleData& cityStyle = gGameMap.mStyleData;

    debug_assert(cityStyle.IsLoaded());
    cxx::ensure_path_exists(outputLocation);
    cxx::string_buffer_1024 pathBuffer;

    SpriteStyle& sprite = cityStyle.mSprites[spriteIndex];

    PixelsArray spriteBitmap;
    spriteBitmap.Create(eTextureFormat_RGBA8, sprite.mWidth, sprite.mHeight, gMemoryManager.mFrameHeapAllocator);
    for (int idelta = 0; idelta < sprite.mDeltaCount; ++idelta)
    {
        if (!cityStyle.GetSpriteTexture(spriteIndex, BIT(idelta), &spriteBitmap, 0, 0))
        {
            debug_assert(false);
        }

        // dump to file
        pathBuffer.printf("%s/sprite_%d_delta_%d.png", outputLocation, spriteIndex, idelta);
        if (!spriteBitmap.SaveToFile(pathBuffer.c_str()))
        {
            debug_assert(false);
        }
    } // for
}

void SpriteManager::FlushSpritesCache()
{
    // move all textures to pool
    for (SpriteCacheElement& currElement: mSpritesCache)
    {
        mFreeSpriteTextures.push_back(currElement.mTexture);
    }

    mSpritesCache.clear();
}

void SpriteManager::FlushSpritesCache(GameObjectID_t objectID)
{
    for (auto icurrent = mSpritesCache.begin(); icurrent != mSpritesCache.end(); )
    {
        if (icurrent->mObjectID == objectID)
        {
            // move texture to pool
            mFreeSpriteTextures.push_back(icurrent->mTexture);

            icurrent = mSpritesCache.erase(icurrent);
            continue;
        }
        ++icurrent;
    }
}

void SpriteManager::DestroySpriteTextures()
{
    for (GpuTexture2D* currTexture: mFreeSpriteTextures)
    {
        gGraphicsDevice.DestroyTexture(currTexture);
    }
    mFreeSpriteTextures.clear();
}

void SpriteManager::GetSpriteTexture(GameObjectID_t objectID, int spriteIndex, SpriteDeltaBits_t deltaBits, Sprite2D& sourceSprite)
{
    sourceSprite.mTexture = nullptr;
    if (deltaBits == 0)
    {
        GetSpriteTexture(objectID, spriteIndex, sourceSprite);
        return;
    }

    debug_assert(spriteIndex < (int) mObjectsSpritesheet.mEntries.size());

    // filter out present delta bits
    SpriteStyle& spriteStyle = gGameMap.mStyleData.mSprites[spriteIndex];
    deltaBits &= spriteStyle.GetDeltaBits();

    if (deltaBits == 0)
    {
        GetSpriteTexture(objectID, spriteIndex, sourceSprite);
        return;
    }

    // find sprite with deltas within cache
    for (SpriteCacheElement& currElement: mSpritesCache)
    {
        if (currElement.mObjectID == objectID && currElement.mSpriteIndex == spriteIndex)
        {
            if (currElement.mSpriteDeltaBits == deltaBits)
            {
                sourceSprite.mTexture = currElement.mTexture;
                sourceSprite.mTextureRegion = currElement.mTextureRegion;
                return;
            }
            currElement.mSpriteDeltaBits = deltaBits;

            // upload changes
            PixelsArray pixels;
            if (!pixels.Create(currElement.mTexture->mFormat, 
                currElement.mTexture->mSize.x, 
                currElement.mTexture->mSize.y, gMemoryManager.mFrameHeapAllocator))
            {
                debug_assert(false);
            }

            if (!gGameMap.mStyleData.GetSpriteTexture(spriteIndex, deltaBits, &pixels, 0, 0))
            {
                debug_assert(false);
            }
            sourceSprite.mTextureRegion = currElement.mTextureRegion;
            sourceSprite.mTexture = currElement.mTexture;
            sourceSprite.mTexture->Upload(pixels.mData);
            return;
        }
    }
    
    // cache miss
    Size2D dimensions;
    dimensions.x = cxx::get_next_pot(spriteStyle.mWidth);
    dimensions.y = cxx::get_next_pot(spriteStyle.mHeight);

    sourceSprite.mTexture = GetFreeSpriteTexture(dimensions, eTextureFormat_RGBA8);
    if (sourceSprite.mTexture == nullptr)
    {
        debug_assert(false);
    }

    PixelsArray pixels;
    if (!pixels.Create(eTextureFormat_RGBA8, dimensions.x, dimensions.y, 
        gMemoryManager.mFrameHeapAllocator))
    {
        debug_assert(false);
    }

    pixels.FillWithCheckerBoard();

    // combine soruce image with deltas
    if (!gGameMap.mStyleData.GetSpriteTexture(spriteIndex, deltaBits, &pixels, 0, 0))
    {
        debug_assert(false);
    }

    // upload to texture
    sourceSprite.mTexture->Upload(pixels.mData);

    Rect2D srcRect;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = spriteStyle.mWidth;
    srcRect.h = spriteStyle.mHeight;

    sourceSprite.mTextureRegion.SetRegion(srcRect, dimensions);

    // add to sprites cache
    SpriteCacheElement spriteCacheElement;
    spriteCacheElement.mObjectID = objectID;
    spriteCacheElement.mSpriteIndex = spriteIndex;
    spriteCacheElement.mSpriteDeltaBits = deltaBits;
    spriteCacheElement.mTexture = sourceSprite.mTexture;
    spriteCacheElement.mTextureRegion = sourceSprite.mTextureRegion;

    mSpritesCache.push_back(spriteCacheElement);
}

void SpriteManager::GetSpriteTexture(GameObjectID_t objectID, int spriteIndex, Sprite2D& sourceSprite)
{
    debug_assert(spriteIndex < (int) mObjectsSpritesheet.mEntries.size());

    sourceSprite.mTexture = mObjectsSpritesheet.mSpritesheetTexture;
    sourceSprite.mTextureRegion = mObjectsSpritesheet.mEntries[spriteIndex];
}

GpuTexture2D* SpriteManager::GetFreeSpriteTexture(const Size2D& dimensions, eTextureFormat format)
{
    for (auto icurr = mFreeSpriteTextures.begin(); icurr != mFreeSpriteTextures.end(); )
    {
        GpuTexture2D* currTexture = *icurr;
        if (currTexture->mSize == dimensions && currTexture->mFormat == format)
        {
            mFreeSpriteTextures.erase(icurr);
            return currTexture;
        }
        ++icurr;
    }

    GpuTexture2D* texture = gGraphicsDevice.CreateTexture2D(format, dimensions.x, dimensions.y, nullptr);
    return texture;
}
