#include "stdafx.h"
#include "StyleData.h"

//////////////////////////////////////////////////////////////////////////

// read distance in map units and convert it to meters
inline bool ParseMapUnits(cxx::json_document_node node, const std::string& attribute, float& output)
{
    cxx::json_document_node attribute_node = node[attribute];
    if (cxx::json_node_numeric numeric_node = attribute_node)
    {
        output = Convert::MapUnitsToMeters(numeric_node.get_value_float());
        return true;
    }
    return false;
}

// read gameobject flags
inline bool ParseObjectFlags(cxx::json_document_node node, const std::string& attribute, GameObjectFlags& flags)
{
    if (cxx::json_node_array flagsNode = node[attribute])
    {
        for (cxx::json_node_string currFlag = flagsNode.first_child();
            currFlag; currFlag = currFlag.next_sibling())
        {
            std::string flag_string = currFlag.get_value();
            if (flag_string == "invisible")
            {
                flags = (flags | GameObjectFlags_Invisible);
                continue;
            }

            if (flag_string == "carobject")
            {
                flags = (flags | GameObjectFlags_CarPart);
                continue;
            }

            debug_assert(false);
        }
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

enum 
{
    GTA_G24FILE_VERSION_CODE = 336,
    GTA_SPRITE_PAGE_DIMS = 256,
    GTA_SPRITE_PAGE_SIZE = GTA_SPRITE_PAGE_DIMS * GTA_SPRITE_PAGE_DIMS,
};

// G24 Header Format

struct GTAFileHeaderG24
{
    unsigned int version_code;
    unsigned int side_size;
    unsigned int lid_size;
    unsigned int aux_size;
    unsigned int anim_size;
    unsigned int clut_size; // total bytes of clut data ( before paging )
    unsigned int tileclut_size;
    unsigned int spriteclut_size;
    unsigned int newcarclut_size;
    unsigned int fontclut_size;
    unsigned int palette_index_size;
    unsigned int object_info_size;
    unsigned int car_size;
    unsigned int sprite_info_size;
    unsigned int sprite_graphics_size;
    unsigned int sprite_numbers_size;
};

//////////////////////////////////////////////////////////////////////////

StyleData::StyleData(): mBlockTexturesRaw(), mPaletteIndices()
    , mLidBlocksCount(), mSideBlocksCount()
    , mAuxBlocksCount(), mTileClutsCount()
    , mSpriteClutsCount(), mRemapClutsCount()
    , mFontClutsCount()
{
    for (int isprite = 0; isprite < CountOf(mSpriteNumbers); ++isprite)
    {
        mSpriteNumbers[isprite] = 0;
    }
}

int StyleData::GetBlockTextureLinearIndex(eBlockType blockType, int blockIndex) const
{
    switch (blockType)
    {
        case eBlockType_Side: 
        {
            debug_assert(blockIndex < mSideBlocksCount);
            return blockIndex;
        }
        case eBlockType_Lid:
        {
            debug_assert(blockIndex < mLidBlocksCount);
            return blockIndex + mSideBlocksCount;
        }
        case eBlockType_Aux:
        {
            debug_assert(blockIndex < mAuxBlocksCount);
            return blockIndex + mSideBlocksCount + mLidBlocksCount;
        }
        default: break;
    }
    debug_assert(false);
    return 0;
}

bool StyleData::LoadFromFile(const std::string& stylesName)
{
    Cleanup();

    std::ifstream file;
    if (!gFiles.OpenBinaryFile(stylesName, file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open style file '%s'", stylesName.c_str());
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read header
    GTAFileHeaderG24 header;
    if (!cxx::read_from_stream(file, header) || header.version_code != GTA_G24FILE_VERSION_CODE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read header of style file '%s'", stylesName.c_str());
        return false;
    }

    debug_assert(header.side_size % MAP_BLOCK_TEXTURE_AREA == 0);
    mSideBlocksCount = header.side_size / MAP_BLOCK_TEXTURE_AREA;

    debug_assert(header.lid_size % MAP_BLOCK_TEXTURE_AREA == 0);
    mLidBlocksCount = header.lid_size / MAP_BLOCK_TEXTURE_AREA;

    debug_assert(header.aux_size % MAP_BLOCK_TEXTURE_AREA == 0);
    mAuxBlocksCount = header.aux_size / MAP_BLOCK_TEXTURE_AREA;

    // various cluts
    mTileClutsCount = header.tileclut_size / sizeof(Palette256);
    mSpriteClutsCount = header.spriteclut_size / sizeof(Palette256);
    mRemapClutsCount = header.newcarclut_size / sizeof(Palette256);
    mFontClutsCount = header.fontclut_size / sizeof(Palette256);

    // clut_size, rounded up to 64K
    int clutsDataLength = cxx::round_up_to(header.clut_size, 64 * 1024);

    if (!ReadBlockTextures(file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read block textures from style file '%s'", stylesName.c_str());
        return false;
    }

    // read the sprite numbers first
    std::streampos currentPos = file.tellg();

    std::streamoff spriteNumbersDataOffset = clutsDataLength + 
        header.anim_size + 
        header.palette_index_size + 
        header.object_info_size + 
        header.car_size +
        header.sprite_info_size + 
        header.sprite_graphics_size;

    file.seekg(spriteNumbersDataOffset, std::ios::cur);

    if (!ReadSpriteNumbers(file, header.sprite_numbers_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read sprite numbers from style file '%s'", stylesName.c_str());
        return false;
    }

    file.seekg(currentPos);

    if (!ReadAnimations(file, header.anim_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read animations from style file '%s'", stylesName.c_str());
        return false;
    }

    if (!ReadCLUTs(file, clutsDataLength))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read palette data from style file '%s'", stylesName.c_str());
        return false;
    }

    if (!ReadPaletteIndices(file, header.palette_index_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read palette indices data from style file '%s'", stylesName.c_str());
        return false;
    }

    if (!ReadObjects(file, header.object_info_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read objects data from style file '%s'", stylesName.c_str());
        return false;
    }

    if (!ReadVehicles(file, header.car_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read cars data from style file '%s'", stylesName.c_str());
        return false;
    }

    if (!ReadSprites(file, header.sprite_info_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read sprites info from style file '%s'", stylesName.c_str());
        return false;
    }

    if (!ReadSpriteGraphics(file, header.sprite_graphics_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read sprite graphics from style file '%s'", stylesName.c_str());
        return false;
    }

    file.close();

    if (!InitGameObjects())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Fail to initialize game objects");
    }

    ReadPedestrianAnimations();
    ReadWeaponTypes();
    // do some data verifications before go further
    if (!DoDataIntegrityCheck())
    {
        debug_assert(false);
    }
    return true;
}

bool StyleData::DoDataIntegrityCheck() const
{
    bool allChecksPassed = true;

    int expectRemapsClutCount = mVehicles.size() * MAX_CAR_REMAPS + MAX_PED_REMAPS;
    if (expectRemapsClutCount != mRemapClutsCount)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Miscalculation for RemapClutsCount (expect %d but read %d)", expectRemapsClutCount, mRemapClutsCount);
        allChecksPassed = false;
    }

    int expectTilesClutCount = GetBlockTexturesCount() * 4;
    if (expectTilesClutCount != mTileClutsCount)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Miscalculation for TileClutsCount (expect %d but read %d)", expectTilesClutCount, mTileClutsCount);
        allChecksPassed = false;
    }

    return allChecksPassed;
}

void StyleData::Cleanup()
{
    mObjectsRaw.clear();
    mWeaponTypes.clear();
    mBlockTexturesRaw.clear();
    mPaletteIndices.clear();
    mPalettes.clear();
    mBlocksAnimations.clear();
    mVehicles.clear();
    mObjects.clear();
    mSprites.clear();
    mSpriteGraphicsRaw.clear();
    mLidBlocksCount = 0;
    mSideBlocksCount = 0;
    mAuxBlocksCount = 0;
    mTileClutsCount = 0;
    mSpriteClutsCount = 0;
    mRemapClutsCount = 0;
    mFontClutsCount = 0;
    // reset all sprite numbers
    for (int isprite = 0; isprite < CountOf(mSpriteNumbers); ++isprite)
    {
        mSpriteNumbers[isprite] = 0;
    }
}

bool StyleData::IsLoaded() const
{
    return (mLidBlocksCount + mSideBlocksCount + mAuxBlocksCount) > 0;
}

bool StyleData::GetBlockAnimationInfo(eBlockType blockType, int blockIndex, BlockAnimationInfo* animationInfo)
{
    debug_assert(animationInfo);
    for (const BlockAnimationInfo& currAnim: mBlocksAnimations)
    {
        if (currAnim.mBlock == blockIndex && currAnim.mWhich == blockType)
        {
            *animationInfo = currAnim;
            return true;
        }
    }
    // not an error
    return false;
}

bool StyleData::HasBlockAnimation(eBlockType blockType, int blockIndex) const
{
    for (const BlockAnimationInfo& currAnim: mBlocksAnimations)
    {
        if (currAnim.mBlock == blockIndex && currAnim.mWhich == blockType)
            return true;
    }
    return false;
}

int StyleData::GetBlockTexturePaletteIndex(eBlockType blockType, int blockIndex, int remap) const
{
    const int blockLinearIndex = GetBlockTextureLinearIndex(blockType, blockIndex);
    if (remap > 0)
    {
        debug_assert(remap < 4);
    }
    return mPaletteIndices[4 * blockLinearIndex + remap];
}

int StyleData::GetSpritePaletteIndex(int spriteClut, int remapClut) const
{
    if (remapClut)
    {
        return mPaletteIndices[mTileClutsCount + mSpriteClutsCount + remapClut];
    }

    return mPaletteIndices[mTileClutsCount + spriteClut];
}

int StyleData::GetFontPaletteIndex(int fontClut) const
{
    return mPaletteIndices[mTileClutsCount + mSpriteClutsCount + mRemapClutsCount + fontClut];
}

bool StyleData::GetBlockTexture(eBlockType blockType, int blockIndex, PixelsArray* bitmap, int destPositionX, int destPositionY, int remap)
{
    // target bitmap must be allocated otherwise operation makes no sence
    if (bitmap == nullptr || !bitmap->HasContent())
    {
        debug_assert(false);
        return false;
    }

    const int blockLinearIndex = GetBlockTextureLinearIndex(blockType, blockIndex);

    // check destination point
    if (destPositionX < 0 || destPositionY < 0)
    {
        debug_assert(false);
    }

    if (destPositionX + MAP_BLOCK_TEXTURE_DIMS > bitmap->mSizex ||
        destPositionY + MAP_BLOCK_TEXTURE_DIMS > bitmap->mSizey)
    {
        debug_assert(false);
    }
   
    // tiles data representation in memory:
    //  ____  ____  ____  ____      <- 4 tiles per scanline ie 256 pixels
    // |    ||    ||    ||    |
    // |____||____||____||____|
    //  ____  ____  ____  ____
    // |    ||    ||    ||    |
    // |____||____||____||____|
    // etc

    int blockX = blockLinearIndex % 4;
    int blockY = blockLinearIndex / 4;

    int srcOffset = (blockY * MAP_BLOCK_TEXTURE_AREA * 4) + (blockX * MAP_BLOCK_TEXTURE_DIMS);
    unsigned char* srcPixels = mBlockTexturesRaw.data() + srcOffset;

    int bpp = NumBytesPerPixel(bitmap->mFormat);
    debug_assert(bpp == 3 || bpp == 4 || bpp == 1);

    int palindex = GetBlockTexturePaletteIndex(blockType, blockIndex, remap);

    for (int iy = 0; iy < MAP_BLOCK_TEXTURE_DIMS; ++iy)
    {
        for (int ix = 0; ix < MAP_BLOCK_TEXTURE_DIMS; ++ix)
        {
            int destOffset = (((destPositionY + iy) * bitmap->mSizex) + (ix + destPositionX)) * bpp;
            unsigned char palentry = srcPixels[ix];

            if (bpp == 1) // color index in palette
            {
                bitmap->mData[destOffset + 0] = palentry;
            }
            else // rgb(a) color
            {
                const Color32& color = mPalettes[palindex].mColors[palentry];
                bitmap->mData[destOffset + 0] = color.mR;
                bitmap->mData[destOffset + 1] = color.mG;
                bitmap->mData[destOffset + 2] = color.mB;
                if (bpp == 4)
                {
                    bitmap->mData[destOffset + 3] = (palentry == 0) ? 0x00 : 0xFF;
                }
            }
        }
        srcPixels += 4 * MAP_BLOCK_TEXTURE_DIMS;
    }
    return true;
}

int StyleData::GetBlockTexturesCount(eBlockType blockType) const
{
    switch (blockType)
    {
        case eBlockType_Side: return mSideBlocksCount;
        case eBlockType_Lid: return mLidBlocksCount;
        case eBlockType_Aux: return mAuxBlocksCount;
        default: break;
    }
    debug_assert(false);
    return 0;
}

int StyleData::GetBlockTexturesCount() const
{
    return mSideBlocksCount + mLidBlocksCount + mAuxBlocksCount;
}

bool StyleData::GetSpriteTexture(int spriteIndex, PixelsArray* bitmap, int destPositionX, int destPositionY)
{
    // target texture must be allocated otherwise operation makes no sence
    if (bitmap == nullptr || !bitmap->HasContent())
    {
        debug_assert(false);
        return false;
    }

    const int NumSprites = mSprites.size();
    if (spriteIndex > NumSprites || spriteIndex == NumSprites)
    {
        // not an error
        return false;
    }

    const SpriteInfo& sprite = mSprites[spriteIndex];

    unsigned char* srcPixels = mSpriteGraphicsRaw.data() + GTA_SPRITE_PAGE_SIZE * sprite.mPageNumber;
    int bpp = NumBytesPerPixel(bitmap->mFormat);
    debug_assert(bpp == 3 || bpp == 4 || bpp == 1);
    debug_assert(bitmap->mSizex >= destPositionX + sprite.mWidth);
    debug_assert(bitmap->mSizey >= destPositionY + sprite.mHeight);

    for (int iy = 0; iy < sprite.mHeight; ++iy)
    for (int ix = 0; ix < sprite.mWidth; ++ix)
    {
        int destOffset = (((destPositionY + iy) * bitmap->mSizex) + (ix + destPositionX)) * bpp;
        int srcOffset = ((sprite.mPageOffsetY + iy) * GTA_SPRITE_PAGE_DIMS + (ix + sprite.mPageOffsetX));
        int palindex = mPaletteIndices[sprite.mClut + mTileClutsCount];
        int palentry = srcPixels[srcOffset];
        if (bpp == 1) // color index in palette
        {
            bitmap->mData[destOffset + 0] = palentry;
        }
        else
        {
            const Color32& color = mPalettes[palindex].mColors[palentry];
            bitmap->mData[destOffset + 0] = color.mR;
            bitmap->mData[destOffset + 1] = color.mG;
            bitmap->mData[destOffset + 2] = color.mB;
            if (bpp == 4)
            {
                bitmap->mData[destOffset + 3] = (palentry == 0) ? 0x00 : 0xFF;
            }
        }
    }
    return true;
}

bool StyleData::GetSpriteTexture(int spriteIndex, SpriteDeltaBits deltas, PixelsArray* bitmap, int destPositionX, int destPositionY)
{
    if (!GetSpriteTexture(spriteIndex, bitmap, destPositionY, destPositionY))
        return false;

    SpriteInfo& sprite = mSprites[spriteIndex];
    if (deltas > 0 && sprite.mDeltaCount > 0)
    {
        for (int idelta = 0; idelta < MAX_SPRITE_DELTAS && idelta < sprite.mDeltaCount; ++idelta)
        {
            if ((deltas & BIT(idelta)) == 0)
                continue;

            SpriteInfo::DeltaInfo& delta = sprite.mDeltas[idelta];
            ApplySpriteDelta(sprite, delta, bitmap, destPositionX, destPositionY);
        }
    }
    return true;
}

void StyleData::ApplySpriteDelta(SpriteInfo& sprite, SpriteInfo::DeltaInfo& spriteDelta, PixelsArray* bitmap, int positionX, int positionY)
{
    unsigned char* srcData = mSpriteGraphicsRaw.data() + spriteDelta.mOffset;
    int bpp = NumBytesPerPixel(bitmap->mFormat);
    debug_assert(bpp == 3 || bpp == 4 || bpp == 1);

    const int HeaderSize = 3;
    unsigned int dstPixelOffset = 0;

    for (unsigned short curr_pos = 0; curr_pos < spriteDelta.mSize; )
    {
        debug_assert(curr_pos + HeaderSize < spriteDelta.mSize);

        unsigned short destination_offset = ((unsigned short)srcData[curr_pos + 0] | ((unsigned short) srcData[curr_pos + 1] << 8));
        unsigned char source_length = (unsigned char) srcData[curr_pos + 2];
        debug_assert(source_length > 0);
        curr_pos += HeaderSize;
        debug_assert(curr_pos + source_length <= spriteDelta.mSize);

        // original offsets are specified with expectation that destination buffer have dimensions GTA_SPRITE_PAGE_DIMS x GTA_SPRITE_PAGE_DIMS
        // therefore some additional recomputation is required
        dstPixelOffset += destination_offset;
        int pagex = dstPixelOffset % GTA_SPRITE_PAGE_DIMS;
        int pagey = dstPixelOffset / GTA_SPRITE_PAGE_DIMS;
        debug_assert(pagex < bitmap->mSizex);
        debug_assert(pagey < bitmap->mSizey);
        debug_assert(pagex + source_length <= bitmap->mSizex);
        
        int palindex = mPaletteIndices[sprite.mClut + mTileClutsCount];
        for (int ipixel = 0; ipixel < source_length; ++ipixel)
        {
            int curr_pixel_offset = (pagey * bitmap->mSizex * bpp) + pagex * bpp;
            int palentry = srcData[curr_pos + ipixel];
            if (bpp == 1) // color index in palette
            {
                bitmap->mData[curr_pixel_offset + 0] = palentry;
            }
            else
            {
                const Color32& color = mPalettes[palindex].mColors[palentry];
                bitmap->mData[curr_pixel_offset + 0] = color.mR;
                bitmap->mData[curr_pixel_offset + 1] = color.mG;
                bitmap->mData[curr_pixel_offset + 2] = color.mB;
                if (bpp == 4)
                {
                    bitmap->mData[curr_pixel_offset + 3] = (palentry == 0) ? 0x00 : 0xFF;
                }
            }
            ++pagex;
        }
        dstPixelOffset += source_length;
        curr_pos += source_length;
    }
}

int StyleData::GetSpriteIndex(eSpriteType spriteType, int spriteId) const
{
    debug_assert(spriteType < eSpriteType_COUNT);

    int offset = 0;
    for (int i = 0; i < spriteType; ++i)
    {
        offset += mSpriteNumbers[i];
    }
    return offset + spriteId;
}

int StyleData::GetVehicleSpriteIndex(eVehicleClass vehicleClass, int spriteId) const
{
    debug_assert(vehicleClass < eVehicleClass_COUNT);

    eSpriteType spriteType = eSpriteType_Car;
    switch (vehicleClass)
    {
        case eVehicleClass_Bus: 
                spriteType = eSpriteType_Bus;
            break;
        case eVehicleClass_FrontOfJuggernaut:
        case eVehicleClass_BackOfJuggernaut:
                spriteType = eSpriteType_Car;
            break;
        case eVehicleClass_Motorcycle:
                spriteType = eSpriteType_Bike;
            break;
        case eVehicleClass_StandardCar:
                spriteType = eSpriteType_Car;
            break;
        case eVehicleClass_Train:
                spriteType = eSpriteType_Train;
            break;
        case eVehicleClass_Tram:
                spriteType = eSpriteType_Tram;
            break;
        case eVehicleClass_Boat:
                spriteType = eSpriteType_Boat;
            break;
        case eVehicleClass_Tank:
                spriteType = eSpriteType_Tank;
            break;
        default:
                debug_assert(false); // unknown vtype!
            break;
    }

    return GetSpriteIndex(spriteType, spriteId);
}

bool StyleData::ReadBlockTextures(std::ifstream& file)
{
    const int totalBlocks = (mSideBlocksCount + mLidBlocksCount + mAuxBlocksCount);

    // tile blocks are stored in paged format 256x256 pixels (4x4 tiles)
    // extra space may be added at the end of aux_block so that the total number of  blocks is a multiple of 4 
    int extraBlocks = 0;
    if ((totalBlocks % 4) > 0)
    {
        extraBlocks = 4 - (totalBlocks % 4);
    }

    const int dataLength = (totalBlocks * MAP_BLOCK_TEXTURE_AREA);
    const int extraLength = (extraBlocks * MAP_BLOCK_TEXTURE_AREA);
    mBlockTexturesRaw.resize(dataLength + extraLength);

    if (!file.read(reinterpret_cast<char*>(mBlockTexturesRaw.data()), mBlockTexturesRaw.size()))
        return false;

    return true;
}

bool StyleData::ReadCLUTs(std::ifstream& file, int dataLength)
{
    const int palCount = dataLength / sizeof(Palette256);
    if (palCount == 0)
    {
        debug_assert(false);
        return false;
    }

    mPalettes.resize(palCount);

    // Read palettes.
    // These are stored in 64k pages, with 64 palettes per page. Each 256 bytes contains a row of 64 RGBA entries,
    // one for each of that page's 64 palettes. Every page has 256 rows, one for each entry for each of that
    // page's 64 palettes.

    char colorBuf[64 * 4];

    const int pageCount = dataLength / (64 * sizeof(Palette256));
    for (int ipage = 0; ipage < pageCount; ++ipage)
    for (int ientry = 0; ientry < 256; ++ientry)
    {
        if (!file.read(colorBuf, sizeof(colorBuf)))
            return false;

        for (int ipalette = 0; ipalette < 64; ++ipalette)
        {
            int ci = ipalette * 4;
            mPalettes[ipalette + ipage * 64].mColors[ientry].SetComponents(colorBuf[ci + 2], 
                colorBuf[ci + 1], 
                colorBuf[ci + 0],
                colorBuf[ci + 3]);
        }
    }

    return true;
}

bool StyleData::ReadPaletteIndices(std::ifstream& file, int dataLength)
{
    mPaletteIndices.resize(dataLength / sizeof(unsigned short));
    // read bunch of shorts
    if (!file.read(reinterpret_cast<char*>(mPaletteIndices.data()), dataLength))
        return false;

    return true;
}

bool StyleData::ReadAnimations(std::ifstream& file, int dataLength)
{
    unsigned char numAnimationBlocks = 0;
    if (!cxx::read_from_stream(file, numAnimationBlocks))
        return false;

    for (int ianimation = 0; ianimation < numAnimationBlocks; ++ianimation)
    {
        BlockAnimationInfo animation;

        READ_I8(file, animation.mBlock);
        READ_I8(file, animation.mWhich);
        READ_I8(file, animation.mSpeed);
        READ_I8(file, animation.mFrameCount);

        for (int iframe = 0; iframe < MAX_MAP_BLOCK_ANIM_FRAMES; ++iframe)
        {
            animation.mFrames[iframe] = -1;
        }

        debug_assert(animation.mFrameCount <= MAX_MAP_BLOCK_ANIM_FRAMES);
        for (int iframe = 0; iframe < animation.mFrameCount; ++iframe)
        {
            READ_I8(file, animation.mFrames[iframe]);
        }

        mBlocksAnimations.push_back(animation);
    }
    return true;
}

bool StyleData::ReadObjects(std::ifstream& file, int dataLength)
{
    for (int icurrentObject = 0; dataLength > 0; ++icurrentObject)
    {
        ObjectRawData objectRawData;

        READ_SI32(file, objectRawData.mWidth);
        READ_SI32(file, objectRawData.mHeight);
        READ_SI32(file, objectRawData.mDepth);
        READ_I16(file, objectRawData.mBaseSprite);
        READ_I16(file, objectRawData.mWeight);
        READ_I16(file, objectRawData.mAux);
        READ_I8(file, objectRawData.mStatus);

        int numInto;
        READ_I8(file, numInto);

        dataLength -= 20;

        // some unused stuff
        if (numInto > 0)
        {
            int skipBytes = numInto * sizeof(unsigned short);
            dataLength -= skipBytes;

            if (!file.seekg(skipBytes, std::ios::cur))
                return false;
        }

        mObjectsRaw.push_back(objectRawData);
    }
    debug_assert(dataLength == 0);
    return dataLength == 0;
}

bool StyleData::ReadVehicles(std::ifstream& file, int dataLength)
{
    for (int icurrent = 0; dataLength > 0; ++icurrent)
    {
        const std::streampos startStreamPos = file.tellg();

        VehicleInfo carInfo;
        carInfo.mRemapsBaseIndex = icurrent * MAX_CAR_REMAPS;

        short dims_width;
        short dims_height;
        short dims_depth;

        READ_SI16(file, dims_width);
        READ_SI16(file, dims_height);
        READ_SI16(file, dims_depth);

        carInfo.mDimensions = Convert::PixelsToMeters({dims_width, dims_depth, dims_height});

        short sprNum;
        READ_SI16(file, sprNum);

        READ_SI16(file, carInfo.mWeight);
        READ_SI16(file, carInfo.mMaxSpeed);
        READ_SI16(file, carInfo.mMinSpeed);
        READ_SI16(file, carInfo.mAcceleration);
        READ_SI16(file, carInfo.mBraking);
        READ_SI16(file, carInfo.mGrip);
        READ_SI16(file, carInfo.mHandling);

        for (int iremap = 0; iremap < MAX_CAR_REMAPS; ++iremap)
        {
            READ_I16(file, carInfo.mRemap[iremap].mH);
            READ_I16(file, carInfo.mRemap[iremap].mL);
            READ_I16(file, carInfo.mRemap[iremap].mS);
        }

        // skip 8bit remaps
        if (!file.seekg(MAX_CAR_REMAPS, std::ios::cur))
            return false;

        unsigned char vtype = 0;
        READ_I8(file, vtype);

        eVehicleClass classID = eVehicleClass_COUNT;
        switch (vtype)
        {
            case 0: classID = eVehicleClass_Bus; break;
            case 1: classID = eVehicleClass_FrontOfJuggernaut; break;
            case 2: classID = eVehicleClass_BackOfJuggernaut; break;
            case 3: classID = eVehicleClass_Motorcycle; break; 
            case 4: classID = eVehicleClass_StandardCar; break;
            case 8: classID = eVehicleClass_Train; break;
            case 9: classID = eVehicleClass_Tram; break;
            case 13: classID = eVehicleClass_Boat; break;
            case 14: classID = eVehicleClass_Tank; break;
        };

        debug_assert(classID < eVehicleClass_COUNT);
        carInfo.mClassID = classID;

        int modelId;
        READ_I8(file, modelId);

        carInfo.mSpriteIndex = GetVehicleSpriteIndex(classID, sprNum); 

        // parse model id
        if (!cxx::parse_enum_int(modelId, carInfo.mModelID))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Unknown car model id: %d", modelId);
            debug_assert(false);
        }

        READ_I8(file, carInfo.mTurning);
        READ_I8(file, carInfo.mDamagable);

        for (int ivalue = 0; ivalue < CountOf(carInfo.mValue); ++ivalue)
        {
            READ_I16(file, carInfo.mValue[ivalue]);
        }

        READ_SI8(file, carInfo.mCx);
        READ_SI8(file, carInfo.mCy);
        READ_SI32(file, carInfo.mMoment);

        READ_FIXEDF32(file, carInfo.mMass);
        READ_FIXEDF32(file, carInfo.mThrust);
        READ_FIXEDF32(file, carInfo.mTyreAdhesionX);
        READ_FIXEDF32(file, carInfo.mTyreAdhesionY);
        READ_FIXEDF32(file, carInfo.mHandbrakeFriction);
        READ_FIXEDF32(file, carInfo.mFootbrakeFriction);
        READ_FIXEDF32(file, carInfo.mFrontBrakeBias);

        READ_SI16(file, carInfo.mTurnRatio);
        READ_SI16(file, carInfo.mDriveWheelOffset);
        READ_SI16(file, carInfo.mSteeringWheelOffset);

        READ_FIXEDF32(file, carInfo.mBackEndSlideValue);
        READ_FIXEDF32(file, carInfo.mHandbrakeSlideValue);

        unsigned char convertible;
        READ_I8(file, convertible);

        carInfo.mConvertible = (convertible & 1) == 1;
        carInfo.mExtraDrivingAnim = (convertible & 2) == 2;

        debug_assert(convertible <= 3);

        READ_I8(file, carInfo.mEngine);
	    READ_I8(file, carInfo.mRadio);
	    READ_I8(file, carInfo.mHorn);
	    READ_I8(file, carInfo.mSoundFunction);
	    READ_I8(file, carInfo.mFastChangeFlag);
        READ_SI16(file, carInfo.mDoorsCount);
        
        debug_assert(carInfo.mDoorsCount <= MAX_CAR_DOORS);

        for (int idoor = 0; idoor < carInfo.mDoorsCount; ++idoor)
        {
            READ_SI16(file, carInfo.mDoors[idoor].mRpy);
            READ_SI16(file, carInfo.mDoors[idoor].mRpx);
            READ_SI16(file, carInfo.mDoors[idoor].mObject);
            READ_SI16(file, carInfo.mDoors[idoor].mDelta);
        }
        mVehicles.push_back(carInfo);

        const std::streampos endStreamPos = file.tellg();

        const int infoLength = static_cast<int>(endStreamPos - startStreamPos);
        dataLength -= infoLength;
    }
    debug_assert(dataLength == 0);
    return dataLength == 0;
}

bool StyleData::ReadSprites(std::ifstream& file, int dataLength)
{
    for (; dataLength > 0;)
    {
        const std::streampos startStreamPos = file.tellg();

        SpriteInfo spriteInfo;
        READ_I8(file, spriteInfo.mWidth);
        READ_I8(file, spriteInfo.mHeight);
        READ_I8(file, spriteInfo.mDeltaCount);

        unsigned char ws_dummy;
        READ_I8(file, ws_dummy);
        READ_I16(file, spriteInfo.mSize);
        READ_I16(file, spriteInfo.mClut);
        READ_I8(file, spriteInfo.mPageOffsetX);
        READ_I8(file, spriteInfo.mPageOffsetY);
        READ_I16(file, spriteInfo.mPageNumber);

        // deltas
        for (int idelta = 0; idelta < spriteInfo.mDeltaCount; ++idelta)
        {
            READ_I16(file, spriteInfo.mDeltas[idelta].mSize);
            READ_SI32(file, spriteInfo.mDeltas[idelta].mOffset);
        }
        mSprites.push_back(spriteInfo);

        const std::streampos endStreamPos = file.tellg();

        const int infoLength = static_cast<int>(endStreamPos - startStreamPos);
        dataLength -= infoLength;
    }
    debug_assert(dataLength == 0);
    return dataLength == 0;
}

bool StyleData::ReadSpriteGraphics(std::ifstream& file, int dataLength)
{
    if (dataLength > 0)
    {
        mSpriteGraphicsRaw.resize(dataLength);

        if (!file.read(reinterpret_cast<char*>(mSpriteGraphicsRaw.data()), dataLength))
            return false;
    }

    return true;
}

bool StyleData::ReadSpriteNumbers(std::ifstream& file, int dataLength)
{
    if (dataLength > 0)
    {
        (void) dataLength;
        for (int isprite = 0; isprite < CountOf(mSpriteNumbers); ++isprite)
        {
            READ_I16(file, mSpriteNumbers[isprite]);
        }
    }

    return true;
}

int StyleData::GetNumSprites(eSpriteType spriteType) const
{
    debug_assert(spriteType < eSpriteType_COUNT);
    return mSpriteNumbers[spriteType];
}

bool StyleData::GetPedestrianAnimation(ePedestrianAnimID animationID, SpriteAnimData& animationData) const
{
    if (animationID < ePedestrianAnim_COUNT)
    {
        animationData = mPedestrianAnimations[animationID];
        debug_assert(animationData.GetFramesCount() > 0);
        return true;
    }
    debug_assert(false);
    return false;
}

int StyleData::GetPedestrianRemapsBaseIndex() const
{
    int remapsBaseIndex = (mRemapClutsCount - MAX_PED_REMAPS);
    debug_assert(remapsBaseIndex > -1);

    return remapsBaseIndex;
}

bool StyleData::ReadWeaponTypes()
{
    mWeaponTypes.resize(eWeapon_COUNT);

    cxx::json_document weaponsConfig;
    if (!gFiles.ReadConfig("entities/weapons.json", weaponsConfig))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read 'entities/weapons.json'");
        return false;
    }

    for (cxx::json_node_object currentNode = weaponsConfig.get_root_node().first_child();
        currentNode; currentNode = currentNode.next_sibling())
    {
        WeaponInfo currentWeapon;
        if (!currentWeapon.SetupFromConfg(currentNode))
        {
            debug_assert(false);
            continue;
        }

        mWeaponTypes[currentWeapon.mWeaponID] = currentWeapon;
    }
    return true;
}

void StyleData::ReadPedestrianAnimations()
{
    cxx::json_document configDocument;
    if (!gFiles.ReadConfig("entities/ped_animations.json", configDocument))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load ped animations config");
        return;
    }

    std::string currAnimName;
    for (cxx::json_node_object currentNode = configDocument.get_root_node().first_child();
        currentNode; currentNode = currentNode.next_sibling())
    {
        currAnimName = currentNode.get_element_name();

        // parse id
        ePedestrianAnimID animID = ePedestrianAnim_Null;
        if (!cxx::parse_enum(currAnimName.c_str(), animID))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Unknown ped anim id '%s'", currAnimName.c_str());
            continue;
        }
        SpriteAnimData& animDesc = mPedestrianAnimations[animID];
        animDesc.Deserialize(currentNode);
      
        // convert to absolute sprite index
        for (int iframe = 0; iframe < animDesc.GetFramesCount(); ++iframe)
        {
            SpriteAnimFrame& animFrame = animDesc.mFrames[iframe];
            animFrame.mSprite = GetSpriteIndex(eSpriteType_Ped, animFrame.mSprite);
        }
    }
}

bool StyleData::InitGameObjects()
{
    int rawObjectsCount = (int) mObjectsRaw.size();

    if (rawObjectsCount != GameObjectType_MAX)
    {
        gConsole.LogMessage(eLogMessage_Info, "Found %d gameobjects which is odd, normal value is %d", rawObjectsCount, GameObjectType_MAX);
    }

    cxx::json_document gameobjects_config;
    if (!gFiles.ReadConfig("entities/gta_objects.json", gameobjects_config))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load gta objects config");
        return false;
    }

    cxx::json_node_array arrayNode = gameobjects_config.get_root_node();
    int arrayElements = arrayNode.get_elements_count();

    if (arrayElements != rawObjectsCount)
    {
        gConsole.LogMessage(eLogMessage_Info, "Found %d gameobjects but %d is expected", rawObjectsCount, arrayElements);
    }

    int numElementsToLoad = std::min(arrayElements, rawObjectsCount);

    // default init objects
    mObjects.clear();
    mObjects.resize(GameObjectType_MAX);
    for (int icurr = 0; icurr < numElementsToLoad; ++icurr)
    {
        GameObjectInfo& currObject = mObjects[icurr];
        currObject.mObjectType = icurr;
        currObject.mLifeDuration = 0;

        ObjectRawData& objectRaw = mObjectsRaw[icurr];

        cxx::json_node_object currObjectNode = arrayNode[icurr];
        if (!currObjectNode)
        {
            debug_assert(false);
            continue;
        }

        if (!cxx::json_get_attribute(currObjectNode, "class", currObject.mClassID))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Unknown gameobject classid");
            continue;
        }
 
        int frameCount = 0;
        cxx::json_get_attribute(currObjectNode, "frameCount", frameCount);
        cxx::json_get_attribute(currObjectNode, "drawOrder", currObject.mDrawOrder);

        ParseObjectFlags(currObjectNode, "flags", currObject.mFlags);

        int startSpriteIndex = GetSpriteIndex(eSpriteType_Object, objectRaw.mBaseSprite);

        // as cds says, 
        // Animated objects are a special case. They cannot be involved in collisions and are there for graphical
        //    effect only. The same data structure is used, with the following differences :
        //      * height - stores the number of game cycles per frame
        //      * width - stores the number of frames
        //      * depth - stores a life descriptor ( 0 for infinite, non-zero n for n animation cycles )
        if (objectRaw.mStatus == 5)
        {
            if (objectRaw.mHeight == 0)
            {
                objectRaw.mHeight = frameCount;
            }
            debug_assert(objectRaw.mHeight > 0);
            currObject.mAnimationData.mFrameRate = (GTA_CYCLES_PER_FRAME * 1.0f) / objectRaw.mHeight;
            currObject.mAnimationData.SetFrames(startSpriteIndex, objectRaw.mWidth);
            currObject.mLifeDuration = objectRaw.mDepth;
        }
        else
        {
            if (frameCount > 0)
            {
                currObject.mAnimationData.SetFrames(startSpriteIndex, frameCount);
            }
            // convert object dimensions pixels to meters
            currObject.mHeight = Convert::PixelsToMeters(objectRaw.mHeight);
            currObject.mWidth = Convert::PixelsToMeters(objectRaw.mWidth);
            currObject.mDepth = Convert::PixelsToMeters(objectRaw.mDepth);
        }

        debug_assert(!std::isinf(currObject.mAnimationData.mFrameRate));
    }

    return true;
}

int StyleData::GetWreckedVehicleSpriteIndex(eVehicleClass vehicleClass) const
{
    static unsigned int StandardCarSpriteVariationCounter = 0;
    const unsigned int StandardCarSpriteVariationsCount = 2;

    int spriteID = 0;
    switch (vehicleClass)
    {
        case eVehicleClass_Bus:
        case eVehicleClass_Train:
        case eVehicleClass_Tram:
            spriteID = 3;
        break;

        case eVehicleClass_FrontOfJuggernaut:
        case eVehicleClass_BackOfJuggernaut:
        case eVehicleClass_StandardCar: 
            // tiny randomness
            spriteID = 0 + ((StandardCarSpriteVariationCounter++) % StandardCarSpriteVariationsCount);
        break;

        case eVehicleClass_Motorcycle:
            spriteID = 5;
        break;

        case eVehicleClass_Boat:
            spriteID = 6;
        break;

        case eVehicleClass_Tank:
            spriteID = 4;
        break;

        default:
            debug_assert(false);
        break;
    }
    return GetSpriteIndex(eSpriteType_WBus, spriteID);
}

int StyleData::GetPedestrianRandomRemap(ePedestrianType pedestrianType) const
{
    static unsigned int CivilianRemapIndexVariationCounter = 0;
    switch (pedestrianType)
    {
        case ePedestrianType_Player1: return -1;
        case ePedestrianType_Player2: return 0;
        case ePedestrianType_Player3: return 1;
        case ePedestrianType_Player4: return 2;
        case ePedestrianType_Civilian:
            // todo: find out correct civilian peds indices
            return (CivilianRemapIndexVariationCounter++ % MAX_PED_REMAPS);
        case ePedestrianType_Police: return NO_REMAP;
        case ePedestrianType_Gang: 
        case ePedestrianType_GangLeader:
            return 18;
        case ePedestrianType_Medical: return 0; // todo
        case ePedestrianType_Fireman: return 1; // todo
    }
    return NO_REMAP;
}
