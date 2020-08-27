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
        if (mOriginMode == eOriginMode_Center)
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

    // Compute sprite corner position
    // @param positions: Output points
    void GetCorners(glm::vec2 positions[4]) const;
    void GetMaxRectPoints(glm::vec2 positions[2]) const;

    // Clear sprite data
    void Clear();

public:
    GpuTexture2D* mTexture = nullptr;
    TextureRegion mTextureRegion; 

    glm::vec2 mPosition;
    cxx::angle_t mRotateAngle;

    float mHeight = 0.0f; // z order
    float mScale = MAP_SPRITE_SCALE;

    unsigned short mPaletteIndex = 0;

    // sprite origin mode
    enum eOriginMode: unsigned short
    {
        eOriginMode_TopLeft,
        eOriginMode_Center,
    };
    eOriginMode mOriginMode = eOriginMode_Center;
    eSpriteDrawOrder mDrawOrder = eSpriteDrawOrder_Background;
};

const int Sizeof_Sprite2D = sizeof(Sprite2D);