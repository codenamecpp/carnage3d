#pragma once

#include "GameDefs.h"
#include "WeaponInfo.h"

class PixelsArray;

// this class holds gta style data which get loaded from G24-files
class StyleData final
{
public:
    // readonly
    std::vector<GameObjectInfo> mObjects;
    std::vector<SpriteInfo> mSprites;
    std::vector<VehicleInfo> mVehicles;
    std::vector<BlockAnimationInfo> mBlocksAnimations;
    std::vector<Palette256> mPalettes;
    std::vector<WeaponInfo> mWeapons;

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
    bool LoadFromFile(const std::string& stylesName);
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
    // @param remap: Remap index, can only be used for pedestrian and vehicle sprites
    int GetSpritePaletteIndex(int spriteClut, int remapClut) const;

    // Get palette index for font
    int GetFontPaletteIndex(int fontClut) const;

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
    bool GetBlockAnimationInfo(eBlockType blockType, int blockIndex, BlockAnimationInfo* animationInfo);
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
    bool GetSpriteTexture(int spriteIndex, SpriteDeltaBits deltas, PixelsArray* bitmap, int destPositionX, int destPositionY);

    // Map sprite type and id pair to sprite index
    // @param spriteType: Sprite type
    // @para spriteId: Sprite id
    int GetSpriteIndex(eSpriteType spriteType, int spriteId) const;

    // Map car vtype and model identifier to sprite index
    // @param vehicleClass: Vehicle class identifier
    // @para spriteId: Sprite id
    int GetVehicleSpriteIndex(eVehicleClass vehicleClass, int spriteId) const;

    // Get sprite index for wrecked car of cpecific vtype
    // @param vehicleClass: Vehicle class identifier
    int GetWreckedVehicleSpriteIndex(eVehicleClass vehicleClass) const;

    // Get number of sprites for specific type 
    // @param spriteType: Sprite type
    int GetNumSprites(eSpriteType spriteType) const;

    // Read speicic sprite animation data
    // @param animationID: Animation identifier
    // @param animationData: Output data
    bool GetPedestrianAnimation(ePedestrianAnimID animationID, SpriteAnimData& animationData) const;

    // Get base clut index for pedestrian sprites
    int GetPedestrianRemapsBaseIndex() const;

private:
    // apply single delta on sprite
    void ApplySpriteDelta(SpriteInfo& sprite, SpriteInfo::DeltaInfo& spriteDelta, PixelsArray* pixelsArray, int positionX, int positionY);

    // Reading style data internals
    // @param file: Source stream
    bool ReadBlockTextures(std::ifstream& file);
    bool ReadCLUTs(std::ifstream& file, int dataLength);
    bool ReadPaletteIndices(std::ifstream& file, int dataLength);
    bool ReadAnimations(std::ifstream& file, int dataLength);
    bool ReadObjects(std::ifstream& file, int dataLength);
    bool ReadVehicles(std::ifstream& file, int dataLength);
    bool ReadSprites(std::ifstream& file, int dataLength);
    bool ReadSpriteGraphics(std::ifstream& file, int dataLength);
    bool ReadSpriteNumbers(std::ifstream& file, int dataLength);

    void ReadPedestrianAnimations();
    void ReadWeapons();

    bool InitGameObjects();

    bool DoDataIntegrityCheck() const;

private:

    struct ObjectRawData
    {
        int mWidth = 0;
        int mHeight = 0;
        int mDepth = 0;
        int mBaseSprite = 0;
        int mWeight = 0;
        int mAux = 0;
        int mStatus = 0; // type
                            // 0 - normal
                            // 1 - ignorable, can drive over
                            // 2 - smashable, breaks on landing
                            // 3 - invisible
                            // 4 - ?
                            // 5 - particle
                            // 6 - carobject
                            // 7 - ?
                            // 8 - scenery, not sure
                            // 9 - powerup
    };

    std::vector<ObjectRawData> mObjectsRaw;

    std::vector<unsigned char> mBlockTexturesRaw;
    std::vector<unsigned char> mSpriteGraphicsRaw;

    // sprites animations
    SpriteAnimData mPedestrianAnimations[ePedestrianAnim_COUNT];

    // counters
    int mTileClutsCount; 
    int mSpriteClutsCount; 
    int mRemapClutsCount;
    int mFontClutsCount;
    int mSideBlocksCount;
    int mLidBlocksCount;
    int mAuxBlocksCount;
    int mSpriteNumbers[eSpriteType_COUNT];
};