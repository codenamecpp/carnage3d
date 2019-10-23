#pragma once

#include "GameDefs.h"

// defines simple 2d sprite
struct Sprite2D
{
public:
    Sprite2D() = default;

    // set origin to center of sprite, texture region and scale must be specified
    inline void SetOriginToCenter()
    {
        mOrigin.x = (-mTextureRegion.mRectangle.w * mScale) * 0.5f;
        mOrigin.y = (-mTextureRegion.mRectangle.h * mScale) * 0.5f;
    }

    // compute corners of the sprite
    // @param points: Output points
    void GetCorners(glm::vec2 points[4]) const;

    // clear sprite data
    void SetNull();

public:
    GpuTexture2D* mTexture = nullptr;
    TextureRegion mTextureRegion; 

    // origin is relative to sprite position and must be set each time texture region or scale changes
    glm::vec2 mOrigin;
    glm::vec2 mPosition;

    float mHeight = 0.0f; // z order
    float mScale = 1.0f;

    cxx::angle_t mRotateAngle;

    unsigned short mClutIndex = 0;
};

const int Sizeof_Sprite2D = sizeof(Sprite2D);