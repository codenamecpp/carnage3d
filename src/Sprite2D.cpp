#include "stdafx.h"
#include "Sprite2D.h"

void Sprite2D::Clear()
{
    mTexture = nullptr;

    mTextureRegion.Clear();

    mOriginMode = eSpriteOrigin_Center;
    mDrawOrder = eSpriteDrawOrder_Background;
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mHeight = 0.0f;
    mScale = MAP_SPRITE_SCALE;
    mPaletteIndex = 0;

    mRotateAngle.set_zero();
}

void Sprite2D::GetCorners(glm::vec2 positions[4]) const
{
    glm::vec2 spriteSize = GetSpriteSize();

    positions[0] = GetOriginPoint();

    positions[1].x = positions[0].x + spriteSize.x;
    positions[1].y = positions[0].y;

    positions[2].x = positions[0].x;
    positions[2].y = positions[0].y + spriteSize.y;

    positions[3].x = positions[1].x;
    positions[3].y = positions[2].y;

    mRotateAngle.to_degrees_normalize_360();
    if (mRotateAngle) // has rotation
    {
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            glm::vec2 posTransformed = glm::rotate(positions[icorner], mRotateAngle.to_radians());
            positions[icorner] = mPosition + posTransformed;
        }
    }
    else
    {
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            positions[icorner] += mPosition;
        } 
    }
}

void Sprite2D::GetApproximateBounds(cxx::aabbox2d_t& bounds) const
{
    glm::vec2 spriteSize = GetSpriteSize();
    glm::vec2 origin;

    float maxsize = std::max(spriteSize.x, spriteSize.y);
    if (mOriginMode == eSpriteOrigin_Center)
    {
        origin.x = origin.y = (-maxsize * 0.5f);
    }
    else
    {
        origin.x = origin.y = 0.0f;
    }

    bounds.mMin = mPosition + origin;
    bounds.mMax.x = (bounds.mMin.x + maxsize);
    bounds.mMax.y = (bounds.mMin.y + maxsize);
}
