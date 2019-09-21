#pragma once

#include "GameDefs.h"

// This class implements caching mechanism for graphic resources

// Since engine uses original GTA assets, cache requires styledata to be provided
// Some textures, such as block tiles, may be combined into huge atlases for performance reasons 
class SpriteManager final: public cxx::noncopyable
{
public:
    // all blocks are packed into single texture array, where each level is single 64x64 bitmap
    GpuTextureArray2D* mBlocksTextureArray = nullptr;

    // all default objects bitmaps (with no deltas applied) are stored in single 2d texture
    Spritesheet mObjectsSpritesheet;

public:
    // preload sprite textures for current level
    bool InitLevelSprites();

    // flush all currently cached sprites
    void Cleanup();

    void DumpBlocksTexture(const char* outputLocation);

private:
    bool InitBlocksTexture();
    bool InitObjectsSpritesheet();
};

extern SpriteManager gSpriteManager;