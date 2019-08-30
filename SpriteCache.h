#pragma once

#include "GameDefs.h"

// This class implements caching mechanism for graphic resources

// Since engine uses original GTA assets, cache requires styledata to be provided
// Some textures, such as block tiles, may be combined into huge atlases for performance reasons 
class SpriteCache final: public cxx::noncopyable
{
public:
    Spritesheet* mBlocksSpritesheet = nullptr;

public:
    // flush all currently cached sprites
    void Cleanup();

    // create city blocks spritesheets, style data must be loaded at this point
    bool CreateBlocksSpritesheet();
    void FreeBlocksSpritesheet();

    // Get block spritesheet entry along with rectangle area where tile is located
    // @param blockType: Block type
    // @param blockIndex: Block index
    // @param spritesheetEntry: Output tile info
    bool GetBlockSpritesheetEntry(eBlockType blockType, int blockIndex, SpritesheetEntry& spritesheetEntry) const;

private:
    Spritesheet* CreateSpritesheet(int picSizex, int picSizey, int picsCount);
    void FreeSpritesheet(Spritesheet* spritesheet);
};

extern SpriteCache gSpriteCache;