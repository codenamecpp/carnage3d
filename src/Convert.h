#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"

// convert fix16 angle to degrees
inline float ConvertFixAngleToDegs(unsigned short ang16)
{
    return (ang16 / 1024.0f) * 360.0f - SPRITE_ZERO_ANGLE;
}

// convert pixels coordinate to map coordinate
inline float ConvertPixelsToMap(int pixels)
{
    return (1.0f * pixels) / MAP_PIXELS_PER_TILE;
}

// convert map coordinate to pixels
inline int ConvertMapToPixels(float coord)
{
    return static_cast<int>(coord * MAP_PIXELS_PER_TILE);
}

// convert map coordinate to physics world coordinate
inline float ConvertMapToPhysics(float coord)
{
    return coord * PHYSICS_SCALE;
}

// convert physics world coordinate to map coordinate
inline float ConvertPhysicsToMap(float coord)
{
    return coord / PHYSICS_SCALE;
}

// convert pixels coordinate to block coordinate
inline int ConvertPixelsToTilePos(int pixels)
{
    return pixels / MAP_PIXELS_PER_TILE;
}

// convert block coordinate to pixels coorinate
inline int ConvertTilePosToPixels(int tilePos)
{
    return tilePos * MAP_PIXELS_PER_TILE;
}

// in original gta1 map height levels is counting from top to bottom - 
// 0 is highest and 5 is lowest level
inline int ConvertMapLevel(int tileLayer)
{
    tileLayer = MAP_LAYERS_COUNT - tileLayer - 1;
    debug_assert(tileLayer > -1 && tileLayer < MAP_LAYERS_COUNT);

    return tileLayer;
}