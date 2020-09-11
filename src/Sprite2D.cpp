#include "stdafx.h"
#include "Sprite2D.h"

void Sprite2D::Clear()
{
    mTexture = nullptr;

    mTextureRegion.Clear();

    mOriginMode = eOriginMode_Center;
    mDrawOrder = eSpriteDrawOrder_Background;
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mHeight = 0.0f;
    mScale = 1.0f;
    mPaletteIndex = 0;

    mRotateAngle.set_zero();
}

bool Sprite2D::IsNull() const
{
    return (mTexture == nullptr);
}

void Sprite2D::GetCorners(glm::vec2 positions[4]) const
{
    float rectw = mTextureRegion.mRectangle.w * mScale;
    float recth = mTextureRegion.mRectangle.h * mScale;

    positions[0] = GetOriginPoint();

    positions[1].x = positions[0].x + rectw;
    positions[1].y = positions[0].y;

    positions[2].x = positions[0].x;
    positions[2].y = positions[0].y + recth;

    positions[3].x = positions[1].x;
    positions[3].y = positions[2].y;

    mRotateAngle.to_degrees_normalize_360();
    if (mRotateAngle.non_zero()) // has rotation
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
    float rectw = mTextureRegion.mRectangle.w * mScale;
    float recth = mTextureRegion.mRectangle.h * mScale;
    float maxsize = std::max(rectw, recth);

    glm::vec2 origin;
    if (mOriginMode == eOriginMode_Center)
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
