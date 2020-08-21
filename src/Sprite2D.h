#pragma once

#include "GameDefs.h"

// defines simple 2d sprite
struct Sprite2D
{
public:
    Sprite2D() = default;

    // Set origin to center of sprite, texture region and scale must be specified
    inline void SetOriginToCenter()
    {
        mOrigin.x = (-mTextureRegion.mRectangle.w * mScale) * 0.5f;
        mOrigin.y = (-mTextureRegion.mRectangle.h * mScale) * 0.5f;
    }
    // Compute corners of the sprite
    // @param positions: Output points
    void GetCorners(glm::vec2 positions[4]) const;
    void GetMaxRectPoints(glm::vec2 positions[2]) const;

    // Clear sprite data
    void Clear();

public:
    GpuTexture2D* mTexture = nullptr;
    TextureRegion mTextureRegion; 

    // origin is relative to sprite position and must be set each time texture region or scale changes
    glm::vec2 mOrigin;
    glm::vec2 mPosition;
    cxx::angle_t mRotateAngle;

    float mHeight = 0.0f; // z order
    float mScale = MAP_SPRITE_SCALE;

    unsigned short mPaletteIndex = 0;
};

const int Sizeof_Sprite2D = sizeof(Sprite2D);