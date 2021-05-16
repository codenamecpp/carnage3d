#pragma once

#include "GameDefs.h"

// defines sprite data
struct Sprite2D
{
public:
    Sprite2D() = default;

    // Compute origin position which is depends on current mode
    inline glm::vec2 GetOriginPoint() const
    {
        glm::vec2 origin;
        if (mOriginMode == eSpriteOrigin_Center)
        {
            origin.x = (-mTextureRegion.mRectangle.w * mScale) * 0.5f;
            origin.y = (-mTextureRegion.mRectangle.h * mScale) * 0.5f;
        }
        else
        {
            origin.x = 0.0f;
            origin.y = 0.0f;
        }
        return origin;
    }

    // Compute size
    inline glm::vec2 GetSpriteSize() const
    {
        return {
            mTextureRegion.mRectangle.w * mScale,
            mTextureRegion.mRectangle.h * mScale
        };
    }

    // Compute sprite corner position
    void GetCorners(glm::vec2 positions[4]) const;

    // Compute very inaccurate yet fast bounding box for sprite
    void GetApproximateBounds(cxx::aabbox2d_t& bounds) const;

    // Clear sprite data
    void Clear();

    // Whether sprite graphics is not specified
    inline operator bool () const { return mTexture != nullptr; }

public:
    GpuTexture2D* mTexture = nullptr;
    TextureRegion mTextureRegion; 

    glm::vec2 mPosition;
    cxx::angle_t mRotateAngle;

    float mHeight = 0.0f; // z order
    float mScale = MAP_SPRITE_SCALE;

    unsigned short mPaletteIndex = 0;

    eSpriteOriginMode mOriginMode = eSpriteOrigin_Center;
    eSpriteDrawOrder mDrawOrder = eSpriteDrawOrder_Background;
};

const int Sizeof_Sprite2D = sizeof(Sprite2D);