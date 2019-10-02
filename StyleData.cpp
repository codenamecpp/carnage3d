#include "stdafx.h"
#include "StyleData.h"

//////////////////////////////////////////////////////////////////////////

template<typename TValue>
inline bool read_from_stream(std::ifstream& filestream, TValue& outputValue)
{
    if (!filestream.read(reinterpret_cast<char*>(&outputValue), sizeof(outputValue)))
        return false;

    return true;
}

// helpers
#define READ_DATA(filestream, destination, datatype) \
    { \
        datatype _$data; \
        if (!read_from_stream(filestream, _$data)) \
            return false; \
        \
        destination = _$data; \
    }

#define READ_I8(filestream, destination) READ_DATA(filestream, destination, unsigned char)
#define READ_I16(filestream, destination) READ_DATA(filestream, destination, unsigned short)
#define READ_I32(filestream, destination) READ_DATA(filestream, destination, int)
#define READ_BOOL(filestream, destination) \
    { \
        unsigned char _$data; \
        if (!read_from_stream(filestream, _$data)) \
            return false; \
        \
        destination = _$data > 0; \
    }
#define READ_FIXEDF32(filestream, destination) \
    { \
        int _$data; \
        if (!read_from_stream(filestream, _$data)) \
            return false; \
        \
        destination = _$data / 65536.0f; \
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
    , mAuxBlocksCount(), mTileClutSize()
    , mSpriteClutSize(), mRemapClutSize()
    , mFontClutSize()
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
    }
    debug_assert(false);
    return 0;
}

bool StyleData::LoadFromFile(const char* stylesName)
{
    Cleanup();

    std::ifstream file;
    if (!gFiles.OpenBinaryFile(stylesName, file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open style file '%s'", stylesName);
        return false;
    }

    // read header
    GTAFileHeaderG24 header;
    if (!read_from_stream(file, header) || header.version_code != GTA_G24FILE_VERSION_CODE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read header of style file '%s'", stylesName);
        return false;
    }

    debug_assert(header.side_size % MAP_BLOCK_TEXTURE_AREA == 0);
    mSideBlocksCount = header.side_size / MAP_BLOCK_TEXTURE_AREA;

    debug_assert(header.lid_size % MAP_BLOCK_TEXTURE_AREA == 0);
    mLidBlocksCount = header.lid_size / MAP_BLOCK_TEXTURE_AREA;

    debug_assert(header.aux_size % MAP_BLOCK_TEXTURE_AREA == 0);
    mAuxBlocksCount = header.aux_size / MAP_BLOCK_TEXTURE_AREA;

    // various cluts
    mTileClutSize = header.tileclut_size;
    mSpriteClutSize = header.spriteclut_size;
    mRemapClutSize = header.newcarclut_size;
    mFontClutSize = header.fontclut_size;

    if (!ReadBlockTextures(file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read block textures from style file '%s'", stylesName);
        return false;
    }

    if (!ReadAnimations(file, header.anim_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read animations from style file '%s'", stylesName);
        return false;
    }

    // clut_size, rounded up to 64K
    int clutsDataLength = cxx::round_up_to(header.clut_size, 64 * 1024);
    if (!ReadCLUTs(file, clutsDataLength))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read palette data from style file '%s'", stylesName);
        return false;
    }

    if (!ReadPaletteIndices(file, header.palette_index_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read palette indices data from style file '%s'", stylesName);
        return false;
    }

    if (!ReadObjects(file, header.object_info_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read objects data from style file '%s'", stylesName);
        return false;
    }

    if (!ReadCars(file, header.car_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read cars data from style file '%s'", stylesName);
        return false;
    }

    if (!ReadSprites(file, header.sprite_info_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read sprites info from style file '%s'", stylesName);
        return false;
    }

    if (!ReadSpriteGraphics(file, header.sprite_graphics_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read sprite graphics from style file '%s'", stylesName);
        return false;
    }

    if (!ReadSpriteNumbers(file, header.sprite_numbers_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read sprite numbers from style file '%s'", stylesName);
        return false;
    }

    InitSpriteAnimations();
    return true;
}

void StyleData::Cleanup()
{
    mBlockTexturesRaw.clear();
    mPaletteIndices.clear();
    mPalettes.clear();
    mBlocksAnimations.clear();
    mCars.clear();
    mObjects.clear();
    mSprites.clear();
    mSpriteGraphicsRaw.clear();
    mLidBlocksCount = 0;
    mSideBlocksCount = 0;
    mAuxBlocksCount = 0;
    mTileClutSize = 0;
    mSpriteClutSize = 0;
    mRemapClutSize = 0;
    mFontClutSize = 0;
    // reset all sprite numbers
    for (int isprite = 0; isprite < CountOf(mSpriteNumbers); ++isprite)
    {
        mSpriteNumbers[isprite] = 0;
    }
    // reset all sprite animations
    for (int ianim = 0; ianim < CountOf(mSpriteAnimations); ++ianim)
    {
        mSpriteAnimations[ianim].SetNull();
    }
}

bool StyleData::IsLoaded() const
{
    return (mLidBlocksCount + mSideBlocksCount + mAuxBlocksCount) > 0;
}

bool StyleData::GetBlockAnimationInfo(eBlockType blockType, int blockIndex, BlockAnimationStyle* animationInfo)
{
    debug_assert(animationInfo);
    for (const BlockAnimationStyle& currAnim: mBlocksAnimations)
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
    for (const BlockAnimationStyle& currAnim: mBlocksAnimations)
    {
        if (currAnim.mBlock == blockIndex && currAnim.mWhich == blockType)
            return true;
    }
    return false;
}

bool StyleData::GetBlockTexture(eBlockType blockType, int blockIndex, PixelsArray* bitmap, int destPositionX, int destPositionY)
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
    debug_assert(bpp == 3 || bpp == 4);

    for (int iy = 0; iy < MAP_BLOCK_TEXTURE_DIMS; ++iy)
    {
        for (int ix = 0; ix < MAP_BLOCK_TEXTURE_DIMS; ++ix)
        {
            int destOffset = (((destPositionY + iy) * bitmap->mSizex) + (ix + destPositionX)) * bpp;
            int palindex = mPaletteIndices[4 * blockLinearIndex];
            int palentry = srcPixels[ix];

            const Color32& color = mPalettes[palindex].mColors[palentry];
            bitmap->mData[destOffset + 0] = color.mR;
            bitmap->mData[destOffset + 1] = color.mG;
            bitmap->mData[destOffset + 2] = color.mB;
            if (bpp == 4)
            {
                bitmap->mData[destOffset + 3] = (palentry == 0) ? 0x00 : 0xFF;
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

    const SpriteStyle& sprite = mSprites[spriteIndex];

    unsigned char* srcPixels = mSpriteGraphicsRaw.data() + GTA_SPRITE_PAGE_SIZE * sprite.mPageNumber;
    int bpp = NumBytesPerPixel(bitmap->mFormat);
    debug_assert(bpp == 3 || bpp == 4);
    debug_assert(bitmap->mSizex >= destPositionX + sprite.mWidth);
    debug_assert(bitmap->mSizey >= destPositionY + sprite.mHeight);

    for (int iy = 0; iy < sprite.mHeight; ++iy)
    for (int ix = 0; ix < sprite.mWidth; ++ix)
    {
        int destOffset = (((destPositionY + iy) * bitmap->mSizex) + (ix + destPositionX)) * bpp;
        int srcOffset = ((sprite.mPageOffsetY + iy) * GTA_SPRITE_PAGE_DIMS + (ix + sprite.mPageOffsetX));
        int palindex = mPaletteIndices[sprite.mClut + mTileClutSize / 1024];
        int palentry = srcPixels[srcOffset];
        const Color32& color = mPalettes[palindex].mColors[palentry];
        bitmap->mData[destOffset + 0] = color.mR;
        bitmap->mData[destOffset + 1] = color.mG;
        bitmap->mData[destOffset + 2] = color.mB;
        if (bpp == 4)
        {
            bitmap->mData[destOffset + 3] = (palentry == 0) ? 0x00 : 0xFF;
        }
    }
    return true;
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

int StyleData::GetCarSpriteIndex(eCarVType carVType, int modelId, int spriteId) const
{
    debug_assert(carVType < eCarVType_COUNT);

    eSpriteType spriteType = eSpriteType_Car;
    switch (carVType)
    {
        case eCarVType_Bus: 
                spriteType = eSpriteType_Bus;
            break;
        case eCarVType_FrontOfJuggernaut:
        case eCarVType_BackOfJuggernaut:
                spriteType = eSpriteType_Car;
            break;
        case eCarVType_Motorcycle:
                spriteType = eSpriteType_Bike;
            break;
        case eCarVType_StandardCar:
                spriteType = eSpriteType_Car;
            break;
        case eCarVType_Train:
                spriteType = eSpriteType_Train;
            break;
        case eCarVType_Tram:
                spriteType = eSpriteType_Tram;
            break;
        case eCarVType_Boat:
                spriteType = eSpriteType_Boat;
            break;
        case eCarVType_Tank:
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
    (void)dataLength;
    unsigned char numAnimationBlocks = 0;
    if (!read_from_stream(file, numAnimationBlocks))
        return false;

    for (int ianimation = 0; ianimation < numAnimationBlocks; ++ianimation)
    {
        BlockAnimationStyle animation;

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
    for (; dataLength > 0;)
    {
        MapObjectStyle objectInfo;

        READ_I32(file, objectInfo.mWidth);
        READ_I32(file, objectInfo.mHeight);
        READ_I32(file, objectInfo.mDepth);
        READ_I16(file, objectInfo.mSpriteIndex);
        READ_I16(file, objectInfo.mWeight);
        READ_I16(file, objectInfo.mAux);
        READ_I8(file, objectInfo.mStatus);

        int numInto;
        READ_I8(file, numInto);

        dataLength -= 20;

        // some unused stuff
        if (numInto > 0)
        {
            int skipBytes = numInto * sizeof(unsigned short);
            if (!file.seekg(skipBytes, std::ios::cur))
                return false;
        }

        mObjects.push_back(objectInfo);
    }
    debug_assert(dataLength == 0);
    return dataLength == 0;
}

bool StyleData::ReadCars(std::ifstream& file, int dataLength)
{
    for (; dataLength > 0;)
    {
        const std::streampos startStreamPos = file.tellg();

        CarStyle carInfo;
        READ_I16(file, carInfo.mWidth);
        READ_I16(file, carInfo.mHeight);
        READ_I16(file, carInfo.mDepth);
        READ_I16(file, carInfo.mSprNum);
        READ_I16(file, carInfo.mWeight);
        READ_I16(file, carInfo.mMaxSpeed);
        READ_I16(file, carInfo.mMinSpeed);
        READ_I16(file, carInfo.mAcceleration);
        READ_I16(file, carInfo.mBraking);
        READ_I16(file, carInfo.mGrip);
        READ_I16(file, carInfo.mHandling);

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
        switch(vtype)
        {
            case 0: carInfo.mVType = eCarVType_Bus; break;
            case 1: carInfo.mVType = eCarVType_FrontOfJuggernaut; break;
            case 2: carInfo.mVType = eCarVType_BackOfJuggernaut; break;
            case 3: carInfo.mVType = eCarVType_Motorcycle; break; 
            case 4: carInfo.mVType = eCarVType_StandardCar; break;
            case 8: carInfo.mVType = eCarVType_Train; break;
            case 9: carInfo.mVType = eCarVType_Tram; break;
            case 13: carInfo.mVType = eCarVType_Boat; break;
            case 14: carInfo.mVType = eCarVType_Tank; break;
            default: debug_assert(false);
                break;
        };

        READ_I8(file, carInfo.mModel);
        READ_I8(file, carInfo.mTurning);
        READ_I8(file, carInfo.mDamagable);

        for (int ivalue = 0; ivalue < CountOf(carInfo.mValue); ++ivalue)
        {
            READ_I16(file, carInfo.mValue[ivalue]);
        }

        READ_I8(file, carInfo.mCx);
        READ_I8(file, carInfo.mCy);
        READ_I32(file, carInfo.mMoment);

        READ_FIXEDF32(file, carInfo.mRbpMass);
        READ_FIXEDF32(file, carInfo.mG1Thrust);
        READ_FIXEDF32(file, carInfo.mTyreAdhesionX);
        READ_FIXEDF32(file, carInfo.mTyreAdhesionY);
        READ_FIXEDF32(file, carInfo.mHandbrakeFriction);
        READ_FIXEDF32(file, carInfo.mFootbrakeFriction);
        READ_FIXEDF32(file, carInfo.mFrontBrakeBias);

        READ_I16(file, carInfo.mTurnRatio);
        READ_I16(file, carInfo.mDriveWheelOffset);
        READ_I16(file, carInfo.mSteeringWheelOffset);

        READ_FIXEDF32(file, carInfo.mBackEndSlideValue);
        READ_FIXEDF32(file, carInfo.mHandbrakeSlideValue);

        READ_BOOL(file, carInfo.mConvertible);

        READ_I8(file, carInfo.mEngine);
	    READ_I8(file, carInfo.mRadio);
	    READ_I8(file, carInfo.mHorn);
	    READ_I8(file, carInfo.mSoundFunction);
	    READ_I8(file, carInfo.mFastChangeFlag);
        READ_I16(file, carInfo.mDoorsCount);
        
        debug_assert(carInfo.mDoorsCount <= MAX_CAR_DOORS);

        for (int idoor = 0; idoor < carInfo.mDoorsCount; ++idoor)
        {
            READ_I16(file, carInfo.mDoors[idoor].mRpy);
            READ_I16(file, carInfo.mDoors[idoor].mRpx);
            READ_I16(file, carInfo.mDoors[idoor].mObject);
            READ_I16(file, carInfo.mDoors[idoor].mDelta);
        }
        mCars.push_back(carInfo);

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

        SpriteStyle spriteInfo;
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
            READ_I32(file, spriteInfo.mDeltas[idelta].mOffset);
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

bool StyleData::GetSpriteAnimation(eSpriteAnimationID animationID, SpriteAnimationData& animationData) const
{
    debug_assert(animationID < eSpriteAnimation_COUNT);
    if (animationID < eSpriteAnimation_COUNT)
    {
        animationData = mSpriteAnimations[animationID];
        debug_assert(animationData.NonNull());
        return true;
    }
    return false;
}

void StyleData::InitSpriteAnimations()
{
    mSpriteAnimations[eSpriteAnimationID_Ped_Walk].Setup(0, 8);
    mSpriteAnimations[eSpriteAnimationID_Ped_Run].Setup(8, 8);
    mSpriteAnimations[eSpriteAnimationID_Ped_ExitCar].Setup(16, 8);
    mSpriteAnimations[eSpriteAnimationID_Ped_EnterCar].Setup(24, 10);
    mSpriteAnimations[eSpriteAnimationID_Ped_FallLong].Setup(38, 3);
    mSpriteAnimations[eSpriteAnimationID_Ped_SlideUnderTheCar].Setup(41, 1, 1.0f);
    mSpriteAnimations[eSpriteAnimationID_Ped_StandingStill].Setup(98, 1, 1.0f);
    mSpriteAnimations[eSpriteAnimationID_Ped_JumpOntoCar].Setup(91, 3);
    mSpriteAnimations[eSpriteAnimationID_Ped_SlideOnCar].Setup(94, 1);
    mSpriteAnimations[eSpriteAnimationID_Ped_DropOffCarSliding].Setup(95, 2);
    mSpriteAnimations[eSpriteAnimationID_Ped_FallShort].Setup(38, 2, 1.0f);
    mSpriteAnimations[eSpriteAnimationID_Ped_LiesOnFloor].Setup(42, 1, 1.0f);
}
