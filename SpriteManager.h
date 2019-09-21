#pragma once

#include "GameDefs.h"

// This class implements caching mechanism for graphic resources

// Since engine uses original GTA assets, cache requires styledata to be provided
// Some textures, such as block tiles, may be combined into huge atlases for performance reasons 
class SpriteManager final: public cxx::noncopyable
{
public:
    // animating blocks texture indices table
    GpuTexture1D* mBlocksIndicesTable = nullptr;

    // all blocks are packed into single texture array, where each level is single 64x64 bitmap
    GpuTextureArray2D* mBlocksTextureArray = nullptr;

    // all default objects bitmaps (with no deltas applied) are stored in single 2d texture
    Spritesheet mObjectsSpritesheet;

public:
    // preload sprite textures for current level
    bool InitLevelSprites();

    // flush all currently cached sprites
    void Cleanup();

    void RenderFrameBegin();
    void RenderFrameEnd();

    void UpdateBlocksAnimations(Timespan deltaTime);

    // save all blocks textures to hard drive
    void DumpBlocksTexture(const char* outputLocation);
    void DumpSpriteTextures(const char* outputLocation);

private:
    bool InitBlocksIndicesTable();
    bool InitBlocksTexture();
    bool InitObjectsSpritesheet();
    void InitBlocksAnimations();
    bool ProcessBlocksAnimations();

private:
    
    // animation state for blocks sharing specific texture
    struct BlockAnimation
    {
    public:
        int mBlockIndex; // linear
        int mSpeed; // the number of game cycles to display each frame
        int mFrameCount = 0;
        int mFrames[MAX_MAP_BLOCK_ANIM_FRAMES]; // an array of block numbers, linear
        int mCyclesCount;
        int mCurrentFrame = 0;
    };

    std::vector<BlockAnimation> mBlocksAnimations;
    std::vector<unsigned short> mBlocksIndices;
    Timespan mBlocksAnimTime;
};

extern SpriteManager gSpriteManager;