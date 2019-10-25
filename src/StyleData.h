#pragma once

#include "GameDefs.h"

class PixelsArray;

// this class holds gta style data which get loaded from G24-files
class StyleData final
{
public:
    // public for convenience, should not be modified directly
    std::vector<MapObjectStyle> mObjects;
    std::vector<SpriteStyle> mSprites;
    std::vector<CarStyle> mCars;
    std::vector<BlockAnimationStyle> mBlocksAnimations;
    std::vector<Palette256> mPalettes;

    // CLUT data :
    //  tiles
    //  sprites
    //  car remaps
    //  ped remaps
    //  fonts
    std::vector<unsigned short> mPaletteIndices;

public: 
    StyleData();
    // Load style data from specific file, returns false on error
    // @param stylesName: Target file name
    bool LoadFromFile(const char* stylesName);
    void Cleanup();
    bool IsLoaded() const;

    // Read block bitmap to specific location at target texture
    // Block bitmap has fixed dimensions (GTA_BLOCK_TEXTURE_DIMS x GTA_BLOCK_TEXTURE_DIMS)
    // @param blockType: Source block area type
    // @param blockIndex: Source block index
    // @param bitmap: Target bitmap, must be created
    // @param destPositionX, destPositionY: Location within destination texture where block will be placed
    bool GetBlockTexture(eBlockType blockType, int blockIndex, PixelsArray* bitmap, int destPositionX, int destPositionY, int remap);

    // Get palette index for block tile
    // @param remap: Remap index, remapping applies to lids only, 0 means no remap, 1-3 means look up tile remap index
    int GetBlockTexturePaletteIndex(eBlockType blockType, int blockIndex, int remap) const;

    // Get palette index for sprite
    // @param spriteClut: Sprite clut index
    // @param remap: Remap index, can only be used for pedestrian and car sprites
    int GetSpritePaletteIndex(int spriteClut, int remapClut) const;

    // Get number of textures total or for specific block type only
    // @param blockType: Block type
    int GetBlockTexturesCount(eBlockType blockType) const;
    int GetBlockTexturesCount() const;

    // Convert block type and relative index to absolute block index
    // @param blockType: Block type
    // @param blockIndex: Source relative index
    int GetBlockTextureLinearIndex(eBlockType blockType, int blockIndex) const;
    
    // Read animation info for specified block or returns false if block does not have any
    // @param blockType: Source block area type
    // @param blockIndex: Target block index
    // @param animationInfo: Output info
    bool GetBlockAnimationInfo(eBlockType blockType, int blockIndex, BlockAnimationStyle* animationInfo);
    bool HasBlockAnimation(eBlockType blockType, int blockIndex) const;

    // Read sprite bitmap to specific location at target texture
    // @param spriteIndex: Sprite index
    // @param bitmap: Target bitmap, must be created
    // @param destPositionX, destPositionY: Location within destination texture where block will be placed
    bool GetSpriteTexture(int spriteIndex, PixelsArray* bitmap, int destPositionX, int destPositionY);

    // Read sprite with delta delta bitmap to specific location at target texture
    // @param spriteIndex: Sprite index
    // @param deltas: All applied deltas
    // @param bitmap: Target bitmap, must be created
    // @param destPositionX, destPositionY: Location within destination texture where block will be placed
    bool GetSpriteTexture(int spriteIndex, SpriteDeltaBits_t deltas, PixelsArray* bitmap, int destPositionX, int destPositionY);

    // Map sprite type and id pair to sprite index
    // @param spriteType: Sprite type
    // @para spriteId: Sprite id
    int GetSpriteIndex(eSpriteType spriteType, int spriteId) const;

    // Map car vtype and model identifier to sprite index
    // @param carVType: Car vtype
    // @para spriteId: Sprite id
    int GetCarSpriteIndex(eCarVType carVType, int spriteId) const;

    // Get number of sprites for specific type 
    // @param spriteType: Sprite type
    int GetNumSprites(eSpriteType spriteType) const;

    // Read speicic sprite animation data
    // @param animationID: Animation identifier
    // @param animationData: Output data
    bool GetSpriteAnimation(eSpriteAnimationID animationID, SpriteAnimationData& animationData) const;

    // Get base clut index for pedestrian sprites
    int GetPedestrianRemapsBaseIndex() const;

private:
    // apply single delta on sprite
    void ApplySpriteDelta(SpriteStyle& sprite, SpriteStyle::DeltaInfo& spriteDelta, PixelsArray* pixelsArray, int positionX, int positionY);

    // Reading style data internals
    // @param file: Source stream
    bool ReadBlockTextures(std::ifstream& file);
    bool ReadCLUTs(std::ifstream& file, int dataLength);
    bool ReadPaletteIndices(std::ifstream& file, int dataLength);
    bool ReadAnimations(std::ifstream& file, int dataLength);
    bool ReadObjects(std::ifstream& file, int dataLength);
    bool ReadCars(std::ifstream& file, int dataLength);
    bool ReadSprites(std::ifstream& file, int dataLength);
    bool ReadSpriteGraphics(std::ifstream& file, int dataLength);
    bool ReadSpriteNumbers(std::ifstream& file, int dataLength);

    void InitSpriteAnimations();
    bool DoDataIntegrityCheck() const;

private:
    std::vector<unsigned char> mBlockTexturesRaw;
    std::vector<unsigned char> mSpriteGraphicsRaw;
    SpriteAnimationData mSpriteAnimations[eSpriteAnimation_COUNT];

    int mTileClutsCount, mSpriteClutsCount, mRemapClutsCount, mFontClutsCount;
    int mSideBlocksCount, mLidBlocksCount, mAuxBlocksCount;
    int mSpriteNumbers[eSpriteType_COUNT];
};